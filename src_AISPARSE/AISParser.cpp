
#include "AISParser.h"


#include <csv.hpp>

#include <map>
#include <filesystem>
#include <string>
#include <format>

extern std::string LOG;

#ifdef IMGUI_VERSION_NUM
extern std::mutex mtxMapEntityList;
extern std::vector<MapEntity*> m_MapEntityList;
#else
std::mutex mtxMapEntityList;
std::vector<MapEntity*> m_MapEntityList;
#endif


//std::vector<AISVessel*> AISVesselList;
std::vector<AIS_PARSER::KnownVessel*> AIS_PARSER::KnownVesselList;
std::map <int, std::string> AIS_PARSER::MarineIDList;  //Marine Identifier D..

int AIS_PARSER::MsgCounts[27]{};
int AIS_PARSER::MsgFailCounts[27]{};


bool AIS_PARSER::LoadKnownVesselList()
{
    std::string fname = "c:/web_root/KnownVessels.csv";
    //std::string fname = "KnownVessels.csv";

    //Need to check for file existence..
    bool b = std::filesystem::exists(fname);
    if (false == b)
    {
        ////wxLogMessage("Not found: %s", fname.c_str());
        printf("Not found: %s\r\n", fname.c_str());
        return true;
    }


    csv::CSVReader reader(fname.c_str());
    try
    {
        for (csv::CSVRow& row : reader)
        {
            int mmsi = row["MMSI"].get<int>();
            int imo = row["IMO"].get<int>();
            std::string name = row["Name"].get<std::string>();
            std::string callsign = row["CallSign"].get<std::string>();
            int type = row["Type"].get<int>();
            std::string symbol = row["Symbol"].get<std::string>();
            std::string flag = row["Flag"].get<std::string>();
            int a = row["A"].get<int>();
            int b = row["B"].get<int>();
            int c = row["C"].get<int>();
            int d = row["D"].get<int>();
            std::string shipNumber = row["ShipNumber"].get<std::string>();
            std::string SIDC = row["SIDC"].get<std::string>();
            AIS_PARSER::KnownVessel* kv = new AIS_PARSER::KnownVessel(mmsi, imo, name, callsign, type, symbol, flag, a, b, c, d, shipNumber,SIDC); //FFH-339

            KnownVesselList.push_back(kv);

        }
    }
    catch (std::exception e)
    {
        //printf("Exception while loading MID List %s:%s\r\n", fname.c_str(), e.what());
        return true;
    }
    return false;
}

void AIS_PARSER::BuildKnownVesselList() //deprecated
{
    AIS_PARSER::KnownVessel* kv = new AIS_PARSER::KnownVessel(316130000, 0, "Charlettetown", "CGAJ", 35, "a-f-S-C-L-F-F", "Canada", 134, 17, 0, 0, "FFH-339","SFSPXL------CAG");
    KnownVesselList.push_back(kv);
    /*
    kv = new AIS_PARSER::KnownVessel(316138000, 0, "Halifax", "CGAP", 35, "a-f-S-C-L-F-F", "Canada", 134, 17, 0, 0, "FFH-330");
    KnownVesselList.push_back(kv);

    kv = new AIS_PARSER::KnownVessel(316135000, 0, "Toronto", "CGAD", 35, "a-f-S-C-L-F-F", "Canada", 134, 17, 0, 0, "FFH-333");
    KnownVesselList.push_back(kv);

    kv = new AIS_PARSER::KnownVessel(316030879, 9348182, "Asterix", "CFN7327", 35, "a-f-S-N", "Canada", 183, 34, 0, 0, "FFH-339");
    KnownVesselList.push_back(kv);

    kv = new AIS_PARSER::KnownVessel(777220000, 1, "Chicoutimi", "SGVA", 35, "a-f-U-S-C-A", "Canada", 70, 8, 0, 0,"SSK_879");
    KnownVesselList.push_back(kv);
    */
}

AIS_PARSER::KnownVessel* AIS_PARSER::FindKnownVesselByMMSI(int mmsi)
{
    for (auto kv : KnownVesselList)
    {
        if (kv->MMSI == mmsi) return kv;
    }
    return nullptr;
}

bool AIS_PARSER::LoadMIDTable()
{
    std::string fname = "c:/web_root/MaritimeIdentificationDigits.csv";
    //std::string fname = "MaritimeIdentificationDigits.csv";

    //Need to check for file existence..
    bool b = std::filesystem::exists(fname);
    if (false == b)
    {
        ////wxLogMessage("Not found: %s", fname.c_str());
        return true;
    }

    csv::CSVReader reader(fname.c_str());
    try
    {
        for (csv::CSVRow& row : reader)
        {
            int mid = row["Digit"].get<int>();
            std::string al = row["Allocated"].get<std::string>();
            MarineIDList.insert({ mid,al });
        }
    }
    catch (std::exception e)
    {
        //printf("Exception while loading MID List %s:%s\r\n", fname.c_str(), e.what());
        return true;
    }

    LOG += std::format("LoadMIDTable, size {}", MarineIDList.size());
    return false;
}


std::string AIS_PARSER::FindCountryFromMIDCode(int mmsi)
{
    int mid = mmsi / 1000000;
    auto it = MarineIDList.find(mid);
    if (it != MarineIDList.end())
    {
        return it->second;
    }
    return "@";  //use this to indicate no country found
}

char AIS_PARSER::GetHostilityFromMarineID(int mmsi)
{
    int mid = mmsi / 1000000;
    auto it = MarineIDList.find(mid);
    if (it != MarineIDList.end())
    {
        if (it->second == "Canada") return 'f';
        if (0 == it->second.compare(0, 7, "Russian")) return 'h';
        if (0 == it->second.compare(0, 5, "China")) return 'h';
        if (0 == it->second.compare(0, 7, "Liberia")) return 'h';
        if (0 == it->second.compare(0, 5, "Malta")) return 'h';

        return 'n';
    }
    return 'u';  //not hostile so its neutral
}




AISVessel* AIS_PARSER::FindVesselByMMSI(int mmsi)
{
    std::lock_guard<std::mutex> lock(mtxMapEntityList); // Acquires the lock
    for (auto V : m_MapEntityList)
    {
        if (V->trackType != TT_AIS) continue;
        if (AISVessel* v = dynamic_cast<AISVessel*>(V))
        {
            if (v->mmsi == mmsi) return v;
        }
    }
    return nullptr;
}


std::string getSIDC(int mmsi)
{
    std::string SIDC{};

    AIS_PARSER::KnownVessel* kv = AIS_PARSER::FindKnownVesselByMMSI(mmsi);
    if (nullptr != kv) //it's a known vessel so use that known symbol code
    {
        return kv->SIDC;
        //CurCoTMsg->msg_type = kv->symbol;
        //if (b_useKnownVessleCallsign) CurCoTMsg->callsign = kv->callsign;
        //else CurCoTMsg->callsign = kv->name;
    }
    //SIDC = "SUSPXM------CAG";//unknown

    SIDC = "SNSPXM------CAG";

    if (316 == (mmsi) / 1000000) SIDC = "SFSPXM------CAG";

    //hostile
    if (273 == (mmsi) / 1000000) SIDC = "SHSPXM------CAG";
    if (412 == (mmsi) / 1000000) SIDC = "SHSPXM------CAG";
    if (413 == (mmsi) / 1000000) SIDC = "SHSPXM------CAG";
    if (414 == (mmsi) / 1000000) SIDC = "SHSPXM------CAG";
    if (416 == (mmsi) / 1000000) SIDC = "SHSPXM------CAG";
    if (477 == (mmsi) / 1000000) SIDC = "SHSPXM------CAG";
    return SIDC;
}


AISObject* AIS_PARSER::getAISObjectFromAISPayloadString(std::string AISPayload)
{
    switch (AISPayload[0])
    {
    case '1':  // FALLTHROUGH
    {
        MsgCounts[1]++;
        return AIS_PARSER::ParseAIS123_PosReportPayload(AISPayload, 0);
        break;
    }
    case '2':  // FALLTHROUGH
    {
        MsgCounts[2]++;
        return AIS_PARSER::ParseAIS123_PosReportPayload(AISPayload, 0);
        break;
    }
    case '3':  // 1-3: Class A position report.
    {
        MsgCounts[3]++;
        return AIS_PARSER::ParseAIS123_PosReportPayload(AISPayload, 0);
        break;
    }

    case '4':  // FALLTHROUGH - 4 - Basestation report
    case ';':  // 11 - UTC date response
    {
        //return MakeUnique<libais::Ais4_11>(AISPayload.c_str(), fill_bits);
        break;
    }

    case '5':  // 5 - Ship and Cargo
    {
        MsgCounts[5]++;
        return AIS_PARSER::ParseAIS5IdentPayload(AISPayload, 2);
        break;
    }

    
    case '9':  // 9 - SAR Position
    {
        MsgCounts[9]++;
        return AIS_PARSER::ParseAIS9SARAircraft(AISPayload, 2);
        break;
    }


    
    case 'B':  // 18 - Position, Class B
    {
        MsgCounts[18]++;
        return AIS_PARSER::ParseAIS18_PosReportPayload(AISPayload, 0);
        break;
    }


    case 'C':  // 19 - Position and ship, Class B
    {
        //return MakeUnique<libais::Ais19>(body.c_str(), fill_bits);
        break;
    }
    case 'E':  // 21 - Aids to navigation report
        MsgCounts[21]++;
        return AIS_PARSER::ParseAIS21AtoNPayload(AISPayload, 0);
        break;


    case 'H':  // 24 - Static data report
        MsgCounts[24]++;
        return AIS_PARSER::ParseAIS24IdentPayload(AISPayload, 0);
        break;

    /*
        case 'K':  // 27 - Long-range AIS broadcast message
        return MakeUnique<libais::Ais27>(body.c_str(), fill_bits);
    */

    }
    return nullptr;
}


AISObject * AIS_PARSER::ParseAIS5IdentPayload(std::string AISPayload, int fillbits)
{
    std::unique_ptr<libais::AisMsg>  p = CreateAisMsg(AISPayload, 0);
    if (nullptr == p)
    {
        MsgFailCounts[5]++;
        return nullptr;
    }
    else
    {
        Ais5 *a5 = new Ais5(AISPayload.c_str(), fillbits);

        AISVessel* v = AIS_PARSER::FindVesselByMMSI(a5->mmsi);
        if (nullptr == v)
        {
            v = new AISVessel(a5);
            v->mmsi = a5->mmsi;
            v->SIDC = getSIDC(v->mmsi);
            v->callsign = a5->callsign;
            v->name = a5->name;
            v->type_and_cargo = a5->type_and_cargo;
            v->destination = a5->destination;
            v->CountryFromMIDCode = AIS_PARSER::FindCountryFromMIDCode(v->mmsi);
            //AISVesselList.push_back(v);
            std::lock_guard<std::mutex> lock(mtxMapEntityList); // Acquires the lock
            m_MapEntityList.push_back((MapEntity*)v);
        }
        else //just update the thing
        {
            v->ais5 = a5;
            v->callsign = a5->callsign;
            v->name = a5->name;
            v->type_and_cargo = a5->type_and_cargo;
            v->destination = a5->destination;
            v->age = 0;
        }
        return (AISObject*)v;
    }
    return nullptr;
}


AISObject* AIS_PARSER::ParseAIS24IdentPayload(std::string AISPayload, int fillbits)
{
    std::unique_ptr<libais::AisMsg>  p = CreateAisMsg(AISPayload, 0);
    if (nullptr == p)
    {
        MsgFailCounts[24]++;
        return nullptr;
    }
    else
    {
        Ais24* a24 = new Ais24(AISPayload.c_str(), fillbits);

        AISVessel* v = AIS_PARSER::FindVesselByMMSI(a24->mmsi);
        if (nullptr == v)
        {
            v = new AISVessel(a24);
            v->mmsi = a24->mmsi;
            v->SIDC = getSIDC(v->mmsi);
            v->callsign = a24->callsign;
            v->name = a24->name;
            v->type_and_cargo = a24->type_and_cargo;
            //v->destination = a24->destination;
            v->CountryFromMIDCode = AIS_PARSER::FindCountryFromMIDCode(v->mmsi);
            //AISVesselList.push_back(v);
            std::lock_guard<std::mutex> lock(mtxMapEntityList); // Acquires the lock
            m_MapEntityList.push_back((MapEntity*)v);
        }
        else //just update the thing
        {
            v->ais24 = a24;
            v->callsign = a24->callsign;
            v->name = a24->name;
            v->type_and_cargo = a24->type_and_cargo;
            //v->destination = a24->destination;
            v->age = 0;
        }
        return (AISObject*)v;
    }
    return nullptr;
}


AISObject * AIS_PARSER::ParseAIS123_PosReportPayload(std::string AISPayload, int fillbits)
{
    std::unique_ptr<libais::AisMsg>  p = CreateAisMsg(AISPayload, fillbits);
    if (nullptr == p)
    {
        MsgFailCounts[0]++; //Zero means undetermined message 
        return nullptr;
    }
    else
    {
        Ais1_2_3 *a123 =  new Ais1_2_3(AISPayload.c_str(), 0);

        AISVessel* v = AIS_PARSER::FindVesselByMMSI(a123->mmsi);
        if (nullptr == v)
        {
            v = new AISVessel(a123);
            v->mmsi = a123->mmsi;
            v->SIDC = getSIDC(v->mmsi);
            v->nav_status = a123->nav_status;
            v->cog = a123->cog;
            v->sog = a123->sog;
            v->true_heading = a123->true_heading;
            v->EntityLat = a123->position.lat_deg;
            v->EntityLng = a123->position.lng_deg;
            v->timestamp = a123->timestamp;
            v->CountryFromMIDCode = AIS_PARSER::FindCountryFromMIDCode(v->mmsi);
            std::lock_guard<std::mutex> lock(mtxMapEntityList); // Acquires the lock
            m_MapEntityList.push_back((MapEntity *)v);

        }
        else //just update the thing
        {
            v->a123 = a123;
            v->nav_status = a123->nav_status;
            v->cog = a123->cog;
            v->sog = a123->sog;
            v->true_heading = a123->true_heading;
            v->EntityLat = a123->position.lat_deg;
            v->EntityLng = a123->position.lng_deg;
            //printf("LL: %7.4f  %8.4f\r\n", v->lat_deg, v->lng_deg);
            v->timestamp = a123->timestamp;
            v->age = 0;
        }

        ////wxLogMessage("new a123  AISMsgNum: %d", v->AISMsgNumber);

        return (AISObject *)v;
    }
    return nullptr;

}


AISObject * AIS_PARSER::ParseAIS18_PosReportPayload(std::string AISPayload, int fillbits)
{
    std::stringstream retVal{};

    std::unique_ptr<libais::AisMsg>  p = CreateAisMsg(AISPayload, fillbits);
    if (nullptr == p)
    {
        MsgFailCounts[18]++;
        return nullptr;
    }
    else
    {
        Ais18* a18 = new Ais18(AISPayload.c_str(), 0);

        //std::unique_ptr<Ais1_2_3> a123 = std::unique_ptr<Ais1_2_3>(new Ais1_2_3(body.c_str(), 0));
        retVal << "ParsePosReportPayload:" << std::endl;
        retVal << "user ID " << a18->mmsi << std::endl;
        //retVal << "nav_status " << NAV_STATUS[a18->nav_status] << std::endl;
        retVal << "true_heading " << a18->true_heading << std::endl;
        retVal << "position, lat " << a18->position.lat_deg << std::endl;
        retVal << "position, lng " << a18->position.lng_deg << std::endl;
        retVal << "time stamp " << a18->timestamp << std::endl;

        //wxLogMessage(retVal.str().c_str());

        AISVessel* v = AIS_PARSER::FindVesselByMMSI(a18->mmsi);
        if (nullptr == v)
        {
            v = new AISVessel(a18);
            v->mmsi = a18->mmsi;
            v->SIDC = getSIDC(v->mmsi);
            //v->nav_status = -1;// a18->nav_status;
            v->true_heading = a18->true_heading;
            v->EntityLat = a18->position.lat_deg;
            v->EntityLng = a18->position.lng_deg;
            v->timestamp = a18->timestamp;

            v->CountryFromMIDCode = AIS_PARSER::FindCountryFromMIDCode(v->mmsi);
            std::lock_guard<std::mutex> lock(mtxMapEntityList); // Acquires the lock
            m_MapEntityList.push_back((MapEntity*)v);
        }
        else //just update the thing
        {
            v->ais18 = a18;

            //v->nav_status = a18->nav_status;
            v->true_heading = a18->true_heading;
            v->EntityLat = a18->position.lat_deg;
            v->EntityLng = a18->position.lng_deg;
            v->timestamp = a18->timestamp;
            v->age = 0;
        }
        return (AISObject*)v;
    }
    return nullptr;

}


//Aid To Nav
AISObject* AIS_PARSER::ParseAIS21AtoNPayload(std::string AISPayload, int fillbits)
{
    std::unique_ptr<libais::AisMsg>  p = CreateAisMsg(AISPayload, 0);
    if (nullptr == p)
    {
        MsgFailCounts[21]++;
        return nullptr;
    }
    else
    {
        Ais21* a21 = new Ais21(AISPayload.c_str(), fillbits);

        AISVessel* v = AIS_PARSER::FindVesselByMMSI(a21->mmsi);

        //Canadian AtoN MMSI are like: 99316xxxx
        //Physical	99MID1xxx	0	GPS
        //Synthetic	99MID1xxx	0	Surveyed
        //Virtual	99MID6xxx	1	Surveyed
        if (nullptr == v)
        {
            v = new AISVessel(a21);
            v->SIDC = "SNSPN-------CAG";
            v->mmsi = a21->mmsi;
            v->name = a21->name;
            v->CountryFromMIDCode = AIS_PARSER::FindCountryFromMIDCode(v->mmsi);
            v->virtualflag = a21->virtual_aton;
            v->AssignedModeInd = a21->assigned_mode;
            //AISVesselList.push_back(v);
            std::lock_guard<std::mutex> lock(mtxMapEntityList); // Acquires the lock
            m_MapEntityList.push_back((MapEntity*)v);
        }
        else //just update the thing
        {
            v->ais21 = a21;
            v->name = a21->name;
            v->age = 0;
        }
        return (AISObject*)v;
    }
    return nullptr;
}



AISObject* AIS_PARSER::ParseAIS9SARAircraft(std::string AISPayload, int fillbits)
{
    std::unique_ptr<libais::AisMsg>  p = CreateAisMsg(AISPayload, fillbits);
    if (nullptr == p)
    {
        MsgFailCounts[9]++;
        return nullptr;
    }
    else
    {
        Ais9* a9 = new Ais9(AISPayload.c_str(), 0);
        AISVessel* v = new  AISVessel(a9);
        v->true_heading = a9->cog;
        v->cog = a9->cog;
        v->EntityLat = a9->position.lat_deg;
        v->EntityLng = a9->position.lng_deg;
        v->timestamp = a9->timestamp;
        v->CountryFromMIDCode = AIS_PARSER::FindCountryFromMIDCode(v->mmsi);
        v->age = 0;
        return (AISObject*)v;
    }
    return nullptr;
}


void AIS_PARSER::AISSummary()
{
    //printf("AISVesselList size: %d\r\n", (int)AISVesselList.size());
    printf("KnownVesselList size: %d\r\n", (int)KnownVesselList.size());
    printf("MarineIDList size: %d\r\n", (int)MarineIDList.size());

    puts("AIS Parses:");
    printf("AIS ID 1,2,3 \tCount: %d\r\n", AIS_PARSER::MsgCounts[1]);
    printf("AIS ID 5 \tCount: %d\r\n", AIS_PARSER::MsgCounts[5]);
    printf("AIS ID 9 \tCount: %d\r\n", AIS_PARSER::MsgCounts[9]);
    printf("AIS ID 18 \tCount: %d\r\n", AIS_PARSER::MsgCounts[18]);
    printf("AIS ID 21 \tCount: %d\r\n", AIS_PARSER::MsgCounts[21]);
    printf("AIS ID 24 \tCount: %d\r\n", AIS_PARSER::MsgCounts[24]);

    puts("Parse Fails:");
    printf("AIS ID 1,2,3 \tCount: %d\r\n", AIS_PARSER::MsgFailCounts[1]);
    printf("AIS ID 5 \tCount: %d\r\n", AIS_PARSER::MsgFailCounts[5]);
    printf("AIS ID 9 \tCount: %d\r\n", AIS_PARSER::MsgFailCounts[9]);
    printf("AIS ID 18 \tCount: %d\r\n", AIS_PARSER::MsgFailCounts[18]);
    printf("AIS ID 21 \tCount: %d\r\n", AIS_PARSER::MsgFailCounts[21]);
    printf("AIS ID 24 \tCount: %d\r\n", AIS_PARSER::MsgFailCounts[24]);


    //printf("Sent %d CoT messages\r\n", NumCoTSent);
}




std::string AIS_PARSER::getAISPayloadFromNMEA(std::string NMEA_String)
{
    static struct NMEA_AIS_MSG* multipart1{};

    struct NMEA_AIS_MSG* nmeaMsg = new NMEA_AIS_MSG(NMEA_String);
    //if (g_debug) wxLogMessage(nmeaMsg->print().c_str());

    if (1 == nmeaMsg->CountOfFragments)
    {
        multipart1 = nullptr; //standard says that multipart messages must arrive sequentially, so delete the first part if a second first part (or solo) is Rx before the awaited second part
        //wxLogMessage("Non multipart NMEA Msg");
        return nmeaMsg->payload;
    }
    else //it's a multipart message
    {
        if (1 == nmeaMsg->FragmentNumber)
        {
            multipart1 = nmeaMsg;
            //wxLogMessage("multipart NMEA Msg - Frag 1");
            return "";
        }
        else if (2 == nmeaMsg->FragmentNumber)  //standard says that multipart messages must arrive sequentially
        {
            //wxLogMessage("multipart NMEA Msg - Frag 2"); 
            if (nullptr == multipart1)
            {
                //wxLogMessage("multipart Frag 2 but no first part. Discarding");
                return ""; //if Rx a second part but don't have a first part then discard the second part
            }
            nmeaMsg->payload = multipart1->payload + nmeaMsg->payload; //concatenate the payloads
            return nmeaMsg->payload;
        }
        //else if (3 == nmeaMsg->FragmentNumber)  {}  
        //else wxLogMessage("NMEA Msg: Should not get here: multipart message but Fragment Number is %d, not 1 or 2", nmeaMsg->FragmentNumber); 

        return ""; //should never get here
    }
}

