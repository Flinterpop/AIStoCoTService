#pragma once


#include <sstream>
#include <format>
#include <ranges>
#include <algorithm>
#include <vector>
#include <map>
#include <string>

#include "ais.h"            //PART OF LIBAIS
//#include "decode_body.h"    //PART OF LIBAIS

#include "AISVessel.h"


//API
//  std::string getAISPayloadFromNMEA(std::string NMEA_String);
//  AISObject* getAISObjectFromAISPayloadString(std::string body);



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
    extern int MsgCounts[27];
    extern int MsgFailCounts[27];

     
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

            try
            {
                std::string c = fields[6].substr(2);  //caused crash
                isInt = isStringAnInteger(c);
                if (isInt) checksum = std::stoi(c);
                //we don't do anythign with it..
            }
            catch (...)
            {

            }
            

            
            

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
        std::string symbol{}; //SIDC
        std::string ShipNumber{};  //example: FFH-339
        std::string SIDC{};
   

        KnownVessel(int mmsi, int imo, std::string _name, std::string cs, int _TypeOfShip, std::string _symbol, std::string _flag, int a, int b, int c, int d,std::string shipNumber,std::string _SIDC)
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
            SIDC = _SIDC;
        };
    };


  
    bool LoadHostilityList();
    bool LoadKnownVesselList();
    void BuildKnownVesselList();
    bool LoadMIDTable();

    std::string FindCountryFromMIDCode(int mid);
    //char GetHostilityFromMarineID(int mmsi);
    KnownVessel* FindKnownVesselByMMSI(int mmsi);


    AISVessel* FindVesselByMMSI(int mmsi);

    AISObject* getAISObjectFromAISPayloadString(std::string body);
    AISObject* ParseAIS9SARAircraft(std::string body, int fillbits);
    AISObject* ParseAIS123_PosReportPayload(std::string body, int fillbits);
    AISObject* ParseAIS18_PosReportPayload(std::string body, int fillbits);
    AISObject* ParseAIS5IdentPayload(std::string body, int fillbits);
    AISObject* ParseAIS24IdentPayload(std::string body, int fillbits);
    AISObject* ParseAIS21AtoNPayload(std::string body, int fillbits);



    void AISSummary();
    std::string getAISPayloadFromNMEA(std::string NMEA_String);

    extern std::vector<int> HostilityList;
    extern std::vector<AIS_PARSER::KnownVessel*> KnownVesselList;
    extern std::map <int, std::string> MarineIDList;  //Marine Identifier D..



}