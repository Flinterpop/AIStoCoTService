#pragma once

#include <string>
#include <sstream>
#include <functional>

#include "ais.h"
#include "decode_body.h"    //PART OF LIBAIS

#include "AISObject.h"

using namespace libais;

inline constexpr const char* NAV_STATUS[] = { "AIS_NV_STATUS_UNDER_WAY_USING_ENGINE",
    "AIS_NV_STATUS_AT_ANCHOR",
    "AIS_NV_STATUS_NOT_UNDER_COMMAND",
    "AIS_NV_STATUS_RESTRICTED_MANEUVERABILITY",
    "AIS_NV_STATUS_CONSTRAINED_BY_DRAUGHT",
    "AIS_NV_STATUS_MOORED",
    "AIS_NV_STATUS_AGROUND",
    "AIS_NV_STATUS_ENGAGED_IN_FISHING",
    "AIS_NV_STATUS_UNDER_WAY_SAILING",
    "AIS_NV_STATUS_RESERVED1", // reserved for future amendment of navigational status for ships carrying DG, HS, or MP, or IMO hazard or pollutant category C, high-speed craft (HSC)
    "AIS_NV_STATUS_RESERVED2", // reserved for future amendment of navigational status for ships carrying dangerous goods (DG), harmful substances (HS) or marine pollutants (MP), or IMO hazard or pollutant category A, wing in ground (WIG)
    "AIS_NV_STATUS_TOWING_ASTERN", // power-driven vessel towing astern (regional use)
    "AIS_NV_STATUS_PUSHING_AHEAD_OR_TOWING_ALONGSIDE", // power-driven vessel pushing ahead or towing alongside (regional use)
    "AIS_NV_STATUS_RESERVED3", // reserved for future use
    "AIS_NV_STATUS_SART", // AIS-SART (active), MOB-AIS, EPIRB-AIS
    "   ", // undefined = default (also used by AIS-SART, MOB-AIS and EPIRB-AIS under test)
};

inline constexpr const char* NAVAID_TYPE[] = { "Default, Type of Aid to Navigation not specified",
    "Reference point",
    "RACON (radar transponder marking a navigation hazard)",
    "Fixed structure off shore, such as oil platforms, wind farms, rigs.",
    "Spare, Reserved for future use.",
    "Light, without sectors",
    "Light, with sectors",
    "Leading Light Front",
    "Leading Light Rear",
    "Beacon, Cardinal N",
    "Beacon, Cardinal E",
    "Beacon, Cardinal S",
    "Beacon, Cardinal W",
    "Beacon, Port hand",
    "Beacon, Starboard hand",
    "Beacon, Preferred Channel port hand",
    "Beacon, Preferred Channel starboard hand",
    "Beacon, Isolated danger",
    "Beacon, Safe water",
    "Beacon, Special mark",
    "Cardinal Mark N",
    "Cardinal Mark E",
    "Cardinal Mark S",
    "Cardinal Mark W",
    "Port hand Mark",
    "Starboard hand Mark",
    "Preferred Channel Port hand",
    "Preferred Channel Starboard hand",
    "Isolated danger",
    "Safe Water",
    "Special Mark",
    "Light Vessel / LANBY / Rigs"
};

inline constexpr const char* FIX_TYPES[] = {
    "Undefined",
    "GPS",
    "GLONASS",
    "combined GPS/GLONASS",
    "Loran-C",
    "Chayka",
    "integrated navigation system",
    "surveyed",
    "Galileo",
    "Not Used",
    "Not Used",
    "Not Used",
    "Not Used",
    "Not Used",
    "Not Used",
    "internal GNSS",
};


class AISVessel : public AISObject
{
public:

    AISVessel() : AISObject(1, 555)  //dummy for testing
    {
        TT = TT_AIS;
        GUID = mmsi;
        //a123 = 1;
        //isValidAIS123 = true;
    };


    AISVessel(Ais1_2_3* a) : AISObject(a->message_id, a->mmsi)
    {
        TT = TT_AIS;
        GUID = mmsi;
        a123 = a;
        isValidAIS123 = true;
    };
    AISVessel(Ais18* a) : AISObject(a->message_id, a->mmsi)
    {
        TT = TT_AIS;
        GUID = mmsi;
        ais18 = a;
        isValidAIS18 = true;
    };

    AISVessel(Ais5* a) : AISObject(a->message_id, a->mmsi)
    {
        TT = TT_AIS;
        GUID = mmsi;
        ais5 = a;
        isValidAIS5 = true;
    };

    AISVessel(Ais24* a) : AISObject(a->message_id, a->mmsi)
    {
        TT = TT_AIS;
        GUID = mmsi;
        ais24 = a;
        isValidAIS24 = true;
    };

    AISVessel(Ais21* a) : AISObject(a->message_id, a->mmsi)
    {
        TT = TT_AIS;
        GUID = mmsi;
        ais21 = a;
        isValidAIS21 = true;
    };

    AISVessel(Ais9* a) : AISObject(a->message_id, a->mmsi)
    {
        TT = TT_AIS;
        GUID = mmsi;
        ais9 = a;
        isValidAIS9 = true;
    };


    Ais1_2_3* a123{};   //Class A Position Reports
    Ais5* ais5{};       //Class A Ship Data

    Ais18* ais18{};       //Class B Position Reports
    Ais24* ais24{};       //Class B Ship Data

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
    //double lat_deg{};
    //double lng_deg{};
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

            retVal << "nav_status " << NAV_STATUS[nav_status] << std::endl;
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
            retVal << "fix type " << FIX_TYPES[ais5->fix_type] << std::endl;
        }

        if (nullptr != ais21)
        {

            retVal << "navaid Type " << NAVAID_TYPE[NavType] << std::endl;
            retVal << "position, lat " << position.lat_deg << std::endl;
            retVal << "position, lng " << position.lng_deg << std::endl;
            retVal << "time stamp " << timestamp << std::endl;
        }

        return retVal.str();


    }


    //void Draw() override;
    //void DrawSymbol();
    //void TrackBlock();
    //void DrawHooked(bool *);



};
