#pragma once

#include <string>
#include <sstream>
#include <functional>

#include <ImVec2Double.h>
#include "imgui.h"
#include "ImBGUtil.h"

#include "AISVessel.h"


extern bool glob_bPlacards;

extern std::function<ImVec2(double, double)> funcLatLon2VPxy_FP;
bool AISSymbolIsMS2525C  = true;

int TrackDeclutter = 5;
int SymbolSize = 7;
ImU32 AIS_Colour = ImColor(255, 0, 0, 255);


void AISVessel::DrawSymbol()
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImVec2Double loc = funcLatLon2VPxy_FP(EntityLat, EntityLng);
    Entity_mp_x = (int)loc.x;
    Entity_mp_y = (int)loc.y;

    if (AISSymbolIsMS2525C)
        DrawMS2552((MapEntity*)this);
    else
    {


        int symbolSize = SymbolSize;
        if (isHovered) symbolSize = 2 * SymbolSize;
        ImVec2 tri[4];
        tri[0] = ImVec2(0, -symbolSize * 2);
        tri[1] = ImVec2(-symbolSize, symbolSize);
        tri[2] = ImVec2(symbolSize, symbolSize);

        //#define M_PI       3.14159265358979323846   // pi
        constexpr double M_PI = 3.14159265358979323846;   // pi

        double angle_rad = true_heading / (180 / M_PI);
        for (int i = 0; i < 4; i++)
        {
            double new_x = tri[i].x * cos(angle_rad) - tri[i].y * sin(angle_rad);
            double new_y = tri[i].x * sin(angle_rad) + tri[i].y * cos(angle_rad);
            tri[i].x = (float)(new_x + loc.x);
            tri[i].y = (float)(new_y + loc.y);
        }
        tri[3] = tri[0];
        draw_list->AddPolyline(tri, 4, AIS_Colour, 0, 1.0); //AddPolyline(const ImVec2 * points, const int points_count, ImU32 col, ImDrawFlags flags, float thickness)
    }

    //if (b_ShowTrackTrails) DrawTrackTrail(ac);

}

void AISVessel::TrackBlock(int _TrackDeclutter)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    char buf[180];

    switch (_TrackDeclutter)
    {
    case 0:  //Just Call sign
    {
        if (callsign.size() == 0) sprintf(buf, "MMSI:%d", mmsi);
        else sprintf(buf, "%s", callsign.c_str());
        break;
    }
    case 1: //civil ID
    case 3: //Military
    {
        if (callsign.size() == 0) sprintf(buf, "MMSI:%d", mmsi);
        sprintf(buf, "%s", name.c_str());
        break;
    }
    case 2: //dynamics
    case 4: //flight path
    {
        sprintf(buf, "AIS - Live");
        break;
    }
    case 5: //Link 16
    default:
        sprintf(buf, "%s\r\n%d\r\n%s\r\n%3.0fkts @ %3.0fT\r\n%s\r\nhooked:%c", CountryFromMIDCode.c_str(), mmsi, callsign.c_str(), sog, cog, SIDC.c_str(),isHooked ? 'T' : 'F');
        break;//draw nothing
    }
    //draw_list->AddText(ImVec2(loc.x + 10, loc.y - 10), ADSB_AC_Colour, buf);




    if (false == glob_bPlacards) draw_list->AddText(ImVec2(Entity_mp_x + 20, Entity_mp_y - 20), AIS_Colour, buf);
    else
    {
        ImU32 symbolColor = ImColor(0, 255, 255, 255);
        DrawBoxedText(buf, ImVec2(Entity_mp_x + 20, Entity_mp_y - 20), symbolColor, symbolColor, IM_COL32(16, 16, 58, 151));

        /*
        if (isHovered)
        {
            if (isHooked) DrawBoxedText(buf, ImVec2(Entity_mp_x + 20, Entity_mp_y - 20), AIS_Colour, ImColor(0, 255, 0, 255));
            else DrawBoxedText(buf, ImVec2(Entity_mp_x + 20, Entity_mp_y - 20), AIS_Colour, ImColor(255, 0, 0, 255));
        }
        else DrawBoxedText(buf, ImVec2(Entity_mp_x + 20, Entity_mp_y - 20), AIS_Colour, ImColor(0, 0, 0, 255));
        */
    }


}


void AISVessel::DrawHooked(bool *pOpen)
{
    std::string label = std::format("AIS Track - {}",mmsi);
    ImGui::Begin(label.c_str(), pOpen);
    label = std::format("MMSI: {}", mmsi);
    ImGui::Text(label.c_str());
    ImGui::Text(name.c_str());
    ImGui::Text(callsign.c_str());
    ImGui::Text(CountryFromMIDCode.c_str());
    ImGui::Text(destination.c_str());

    label = std::format("{:7.4f} {:8.4f}", EntityLat, EntityLng);
    ImGui::Text(label.c_str());

    ImGui::End();


}


void AISVessel::Draw()
{
    DrawSymbol();
    
    if (isHooked) DrawHooked(&isHooked);
    else if (isHovered) TrackBlock();
    else TrackBlock(0);

  
 


};

