#pragma once

//public items:
//some structs: AISObject, Vessel, VesselList

// some vars:
//  int MsgCounts[27];
//  int MsgFailCounts[27];

//functions:
// 	AIS_PARSER::LoadMIDTable();
//  AIS_PARSER::LoadKnownVesselList();
//  std::string AIS_PARSER::getAISPayloadFromNMEA(nmea);
//  AISObject* AIS_PARSER::getAISObjectFromAISPayloadString(payload);




#include <sstream>
#include <format>
#include <ranges>
#include <algorithm>
#include <vector>
#include <map>

#include "ais.h"            //PART OF LIBAIS
#include "decode_body.h"    //PART OF LIBAIS


using namespace libais;


/* NMEA sentence max length, including \r\n (chars) */
const int NMEA_MAX_LENGTH = 82;

/* NMEA sentence endings, should be \r\n according the NMEA 0183 standard */
const char NMEA_END_CHAR_1 = '\r';

//#define NMEA_END_CHAR_2		'\n'
const char NMEA_END_CHAR_2 = '\n';

/* NMEA sentence prefix length (num chars), Ex: GPGLL */
const int NMEA_PREFIX_LENGTH = 5;


static inline bool isStringADouble(const std::string& s) 
{
    char* end = nullptr;
    double val = std::strtod(s.c_str(), &end);

    // Check if a conversion was performed (end != s.c_str())
    // and if the entire string was consumed (*end == '\\0')
    // This also needs to handle potential issues like underflow/overflow if necessary
    return end != s.c_str() && *end == '\0';
}


static inline bool isStringAnInteger(const std::string& s)
{
    if (s.empty()) return false;

    char* p;
    // strtol attempts to parse the string as a long integer
    // The second argument, &p, is set to point to the character 
    // where parsing stopped. The third argument is the base (10 for decimal).
    long converted_val = std::strtol(s.c_str(), &p, 10);

    // Check if the pointer 'p' reached the end of the string.
    // Also check for leading whitespace, which strtol ignores by default
    // but may be considered invalid for a strict integer check.
    // The *p == 0 check ensures no non-integer characters (like 'a', '.') remain.
    return (*p == 0);
}



namespace AIS_PARSER
{
    extern const char* NAV_STATUS[];
    extern const char* NAVAID_TYPE[];
    extern const char* FIX_TYPES[];
    extern int MsgCounts[27];
    extern int MsgFailCounts[27];


    std::string getAISPayloadFromNMEA(std::string NMEA_String);

     
    struct NMEA_AIS_MSG
    {
        bool isValid = false;
        std::string parseRecordString{};

        std::string sentence{};  //example !AIVDM,1,1,,A,1Cu?etPjh0J`ej@Ih@B1hQH00000,0*5B
        std::string name{};         //Field 1 should be AIVDM
        int CountOfFragments{};     //Field 2   1 or 2
        int FragmentNumber{};       //Field 3   1 or 2 
        int SequentialMessageID{};  //Field 4   often 0, shoudl be the same for fragments of the same message
        std::string RadioChannel{}; //Field 5   A or B 
        std::string payload{};      //Field 6   string of binary data 
        int fillBits{};             //Field 7 before *
        int checksum{};             //Field 7 after *
            
        NMEA_AIS_MSG(std::string NMEA_Sentence)
        {
            ////wxLogMessage("Parsing %s", NMEA_Sentence);

            std::stringstream retVal{};
            std::vector<std::string> fields;
            auto split_view = NMEA_Sentence | std::ranges::views::split(',');
            for (const auto& view : split_view) fields.push_back(std::string(view.begin(), view.end()));
            //for (const std::string& fields : fields) std::cout << fields << std::endl;
            if (fields.size() != 7)
            {
                parseRecordString = "Incorrect number of fields";
                return;
            }

            retVal << "Num Fields: " << fields.size() << std::endl;
            for (auto &s : fields)
                retVal << s << "//";

            sentence = NMEA_Sentence;
            name = fields[0];

            bool isInt = isStringAnInteger(fields[1]);
            if (isInt) CountOfFragments = std::stoi(fields[1]);

            isInt = isStringAnInteger(fields[2]);
            if (isInt) FragmentNumber = std::stoi(fields[2]);

            isInt = isStringAnInteger(fields[3]);
            if (isInt) SequentialMessageID = std::stoi(fields[3]);

            RadioChannel = fields[4];
            payload = fields[5];

            char FB = fields[6][0];
            fillBits = FB - 0x30;

            std::string c = fields[6].substr(2);

            isInt = isStringAnInteger(c);
            if (isInt) checksum = std::stoi(c);

            parseRecordString = retVal.str();
            isValid = true;
        }

        std::string print()
        {
            std::stringstream retVal{};
            retVal << "AIS NMEA Sentence: " << std::endl;
            retVal << sentence << std::endl;
            retVal << std::format("name: {}\r\n", name);
            retVal << std::format("Num Frags: {}\r\n", CountOfFragments);
            retVal << std::format("Frag Num: {}\r\n", FragmentNumber);
            retVal << std::format("Msg ID: {}\r\n", SequentialMessageID);
            retVal << std::format("RadioChannel: {}\r\n", RadioChannel);
            retVal << std::format("Payload: {}\r\n", payload);
            retVal << std::format("fill bits: {}\r\n", fillBits);
            retVal << std::format("Checksum: {}\r\n", checksum);

            return retVal.str();
        }
    };

    struct KnownVessel
    {
        int MMSI{};
        int IMO{};
        std::string name{};
        std::string callsign{};
        int A{}, B{}, C{}, D{};  //dimensions
        int TypeOfShip{};  //from AIS Message 5
        std::string flag{};
        std::string symbol{};
        std::string ShipNumber{};  //example: FFH-339

        KnownVessel(int mmsi, int imo, std::string _name, std::string cs, int _TypeOfShip, std::string _symbol, std::string _flag, int a, int b, int c, int d,std::string shipNumber)
        {
            MMSI = mmsi;
            IMO = imo;
            name = _name;
            callsign = cs;
            A = a;
            B = b;
            C = c;
            D = d;  //dimensions
            TypeOfShip = _TypeOfShip;
            flag = _flag;  //Country of registration
            symbol = _symbol;
            ShipNumber = shipNumber;
        };
    };



    class AISObject  //base class for all AIS things with an MMSI
    {
    public:
        int mmsi = 0;
        int age = 0;
        bool markForDelete = false;
        int AISMsgNumber = 0; //1 thru 27

    public:
        AISObject(int _AISMsgNum, int _mmsi)
        {
            mmsi = _mmsi;
            AISMsgNumber = _AISMsgNum;
            //MsgCounts[AISMsgNumber]++;
        };

        virtual std::string LogMe() {
            std::stringstream retVal{};
            retVal << "AISObject (base class) " << std::endl;
            retVal << "MMSI " << mmsi << std::endl;
            retVal << "Message ID " << AISMsgNumber << std::endl;
            return retVal.str();
        };
    };

    class Vessel : public AISObject
    {
    public:

        Vessel(Ais1_2_3* a) : AISObject(a->message_id, a->mmsi)
        {
            a123 = a;
            isValidAIS123 = true;
        };
        Vessel(Ais18* a) : AISObject(a->message_id, a->mmsi)
        {
            ais18 = a;
            isValidAIS18 = true;
        };

        Vessel(Ais5* a) : AISObject(a->message_id, a->mmsi)
        {
            ais5 = a;
            isValidAIS5 = true;
        };

        Vessel(Ais24* a) : AISObject(a->message_id, a->mmsi)
        {
            ais24 = a;
            isValidAIS24 = true;
        };

        Vessel(Ais21* a) : AISObject(a->message_id, a->mmsi)
        {
            ais21 = a;
            isValidAIS21 = true;
        };

        Vessel(Ais9* a) : AISObject(a->message_id, a->mmsi)
        {
            ais9 = a;
            isValidAIS9 = true;
        };


        Ais1_2_3* a123{};   //Class A Position Reports
        Ais5* ais5{};       //Class A Ship Data

        Ais18* ais18{};       //Class A Position Reports
        Ais24* ais24{};       //Class A Ship Data

        Ais21* ais21{};

        Ais9* ais9{};

        std::string CountryFromMIDCode{};
        bool isValidAIS123{ false };
        bool isValidAIS5{ false };

        bool isValidAIS18{ false };
        bool isValidAIS24{ false };

        bool isValidAIS21{ false }; //Aid To Nav
        bool isValidAIS9{ false }; //SAR Aircraft

        //AIS 1,2,3, 18
        int position_accuracy{};
        AisPoint position{};
        double lat_deg{};
        double lng_deg{};
        float cog{};  // Degrees.
        float sog{};  //knots
        int true_heading{};
        int timestamp{};
        int special_manoeuvre{};
        bool raim{};
        bool utc_valid{};
        int utc_hour{};
        int utc_min{};

        //AIS 1,2,3
        AIS_NAVIGATIONAL_STATUS nav_status{};


        //AIS 5, 24
        int ais_version{};
        int imo_num{};
        std::string callsign{};
        std::string name{};  //Vessel Names that exceed the AIS’s 20 character limit should be shortened (not truncated) to 15 character - spaces, followed by an underscore{ _ },
        int type_and_cargo{};
        int dim_a{};
        int dim_b{};
        int dim_c{};
        int dim_d{};
        int fix_type{};

        int eta_month{};
        int eta_day{};
        int eta_hour{};
        int eta_minute{};
        float draught{};  // present static draft. m
        std::string destination{};
        int dte{};

        //AIS 24
        int Mothership_MMSI{};

        //AIS 21  (Aid to Nav)
        int NavType{}; //0 thru 31
        int EPFD{};
        bool OnOffInd{};
        int AtoNRegApp{};
        bool virtualflag{};//The Virtual Aid flag is interpreted as follows: 0 = default = real Aid to Navigation at indicated position; 1 = virtual Aid to Navigation simulated by nearby AIS station.
        int AssignedModeInd{};
        std::string extendedName{};


        std::string LogMe() override
        {
            std::stringstream retVal{};
            retVal << "AIS_1_2_3 parse: " << std::endl;
            retVal << "MMSI " << mmsi << std::endl;

            if (nullptr != a123)
            {

                retVal << "nav_status " << AIS_PARSER::NAV_STATUS[nav_status] << std::endl;
                retVal << "true_heading " << true_heading << std::endl;
                retVal << "position, lat " << position.lat_deg << std::endl;
                retVal << "position, lng " << position.lng_deg << std::endl;
                retVal << "time stamp " << timestamp << std::endl;
            }

            if (nullptr != ais5)
            {
                retVal << "callsign " << ais5->callsign << std::endl;
                retVal << "name " << ais5->name << std::endl;
                retVal << "type_and_cargo " << ais5->type_and_cargo << std::endl;
                retVal << "destination " << ais5->destination << std::endl;
                retVal << "fix type " << AIS_PARSER::FIX_TYPES[ais5->fix_type] << std::endl;
            }

            if (nullptr != ais21)
            {

                retVal << "navaid Type " << AIS_PARSER::NAVAID_TYPE[NavType] << std::endl;
                retVal << "position, lat " << position.lat_deg << std::endl;
                retVal << "position, lng " << position.lng_deg << std::endl;
                retVal << "time stamp " << timestamp << std::endl;
            }

            return retVal.str();


        }
    };


     
    bool LoadKnownVesselList();
    void BuildKnownVesselList();
    bool LoadMIDTable();

    std::string FindCountryFromMIDCode(int mid);
    char GetHostilityFrom(int mmsi);
    KnownVessel* FindKnownVesselByMMSI(int mmsi);
    Vessel* FindVesselByMMSI(int mmsi);

    AISObject* getAISObjectFromAISPayloadString(std::string body);
    AISObject* ParseAIS9SARAircraft(std::string body, int fillbits);
    AISObject* ParseAIS123_PosReportPayload(std::string body, int fillbits);
    AISObject* ParseAIS18_PosReportPayload(std::string body, int fillbits);
    AISObject* ParseAIS5IdentPayload(std::string body, int fillbits);
    AISObject* ParseAIS24IdentPayload(std::string body, int fillbits);
    AISObject* ParseAIS21AtoNPayload(std::string body, int fillbits);


    extern std::vector<AIS_PARSER::Vessel*> VesselList;
    extern std::vector<AIS_PARSER::KnownVessel*> KnownVesselList;
    extern std::map <int, std::string> MarineIDList;  //Marine Identifier D..


}