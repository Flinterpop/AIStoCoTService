#pragma once

#include <string>
#include <sstream>
#include "MapEntity.h"

void DrawMS2552(MapEntity* me);


class AISObject : public MapEntity  //base class for all AIS things with an MMSI
{
public:
    int mmsi = 0;
    int age = 0;
    bool markForDelete = false;
    int AISMsgNumber = 0; //1 thru 27

    //bool isHovered = false;
    //int Entity_mp_x{};
    //int Entity_mp_y{};
    //int HoverRadius = 25;

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