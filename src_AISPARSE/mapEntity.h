#pragma once
#include <string>
#include <cmath>   // Recommended for all types in C++

//#include "ImageDX.h"




//extern struct Image;

enum trackType { TT_ADSB, TT_AIS, TT_VMF, TT_COT, TT_SIM };
enum hostility { UNKNOWN = 0, ASSUMED_FRIEND = 1, FRIEND = 2, NEUTRAL = 3, SUSPECT = 4, HOSTILE = 5, UNDEFINED = 6 };

class MapEntity
{

	//static inline int NextGUID = 1;

public:
	int GUID{};
	enum trackType trackType {};

	double EntityLat = 0;
	double EntityLng = 0;

	int Entity_mp_x=0;
	int Entity_mp_y=0;
	int HoverRadius = 25;

	bool bShowThisTracksLabels = false;
	bool isHovered = false;
	bool isHooked = false;
	int NumUpdates = 0;
	bool flashMapSymbol = false;  //similar to isThisAircraftHooked but might be useful as a separete item for later feature
	hostility Ident;
	bool markForDelete = false;
	int TrackAge = 0;

	bool bUseImage = false;
	std::string SIDC{};
	std::string MsgType{}; //CoT



	int hoverCheckCount{};
	
	bool bShowLocateACLine = false; //when true a line from the TrackBlock to AC is shown

	MapEntity(){};
	


#ifdef IMGUI_VERSION_NUM
	virtual void Draw() {};
#endif

	


	virtual void EntityIsClicked() 
	{ 
		isHooked = !isHooked;
	};

	
	virtual MapEntity *checkIsHovered(int x, int y, bool isLeftClicked = false)
	{
		isHovered = false;
		double delta = abs(Entity_mp_x - x) + abs(Entity_mp_y - y);
		if (delta < HoverRadius)
		{
			hoverCheckCount++;
			isHovered = true;
			if (isLeftClicked) isHooked = !isHooked;
			return this;
		}
		return nullptr;
	}
		



};