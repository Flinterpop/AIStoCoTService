#pragma once


//has a single public function:
//bg_TakMessage* AISObjectToCoTMessage(AISObject* ao)

#include "AISParser.h"
#include "bg_TakMessage.h"


using namespace AIS_PARSER;

bool b_useKnownVessleCallsign = true;
bool b_showAtoN = true;


namespace NMEA_AIS2COT
{
	//forward declarations
	static bg_TakMessage* VesselToCoTMessage(AISVessel* v);
	static bg_TakMessage* AtoNToCoTMessage(AISVessel* v);
	static bg_TakMessage* SARAircraftToCoTMessage(AISVessel* v);

	static inline std::string getFixType(int fix_type)
	{
		switch (fix_type)
		{
		case 0:
		case 2:
		case 3:
		case 8:
		{
			return std::string("m-g");
			break;
		}
		case 4: //Loran-C
		case 5: //Chayka (Russian Loran-C type thing)
		{
			return std::string("m-r");
			break;
		}
		case 6: {
			return std::string("m-n");  //ins
			break;
		}

		case 7:  //surveyed. For fixed AtoN and virtual AtoN, the charted position should be used. The accurate position enhances its function as a radar reference target
		{
			return std::string("m-i"); //mensurated
			break;
		}
		case 15:  //Internal GNSS
		{
			return std::string("m-g");
			break;
		}
		//9-14 are not used
		default:
		{
			return std::string("m-i");
			break;
		}
		}
	}


	//public function
	inline bg_TakMessage* AISObjectToCoTMessage(AISObject* ao)
	{
		AISVessel* v = (AISVessel*)ao;
		bg_TakMessage* tm{};
		switch (ao->AISMsgNumber)
		{
			case 1:
			case 2:
			case 3:
			case 18:
			{
				tm = NMEA_AIS2COT::VesselToCoTMessage(v);
				break;
			}
			//case 5: never send a AIS5 by itseld - it has no position info
			//case 24:  //Type 24: Class B Info

			case 9:  //Type 9: SAR Aircraft
			{
				tm = NMEA_AIS2COT::SARAircraftToCoTMessage(v);
				break;
			}

			case 21:  //Type 21: Aid-to-Navigation Report
			{
				tm = NMEA_AIS2COT::AtoNToCoTMessage(v);
				break;
			}
		}
		return tm;
	}

	inline bg_TakMessage *VesselToCoTMessage(AISVessel* v)
	{
		//Class A								Class B
		if ((false == v->isValidAIS123) && (false == v->isValidAIS18)) return nullptr;

		bg_TakMessage *CurCoTMsg = new bg_TakMessage();
		CurCoTMsg->IncludeTakControl = true;

		CurCoTMsg->d_lat = v->EntityLat;
		CurCoTMsg->d_lon = v->EntityLng;
		CurCoTMsg->d_hae = 0;
		if (0 == v->position_accuracy) //low
		{
			CurCoTMsg->d_ce = 100;
			CurCoTMsg->d_le = 100;
		}
		else //high 
		{
			CurCoTMsg->d_ce = 10;
			CurCoTMsg->d_le = 10;
		}

		CurCoTMsg->tm_validTimeInSeconds = 90;

		CurCoTMsg->UID = std::format("MMSI-{}", v->mmsi);
		CurCoTMsg->_how = getFixType(v->fix_type);

		CurCoTMsg->course = v->cog;// should use COG or true_heading??

		//AIS Speed over ground : 0.1 - knot(0.19 km / h) resolution from
		//                   0 to 102 knots(189 km / h)
		// COT Speed is meters / second
		// Pre - computed constant : 0.1 / 1.944 = 0.05144
		CurCoTMsg->speed = v->sog * 0.05144;

		CurCoTMsg->includeContact = true;
		std::string name{};
		AISVessel* v2 = FindVesselByMMSI(v->mmsi);
		if (nullptr != v2)  //found vessel in vessel list
		{
			if (0 != v2->callsign.size())
			{
				std::string cs = v2->callsign;
				std::erase(cs, '@'); // C++20 only
				if (0 != cs.size()) CurCoTMsg->callsign = cs;// v2->callsign;
				else
				{
					std::string cs = v2->name;
					std::erase(cs, '@'); // C++20 only
					if (0 != cs.size()) CurCoTMsg->callsign = cs;
				}
			}
			if (0 != v2->name.size()) name = v2->name;
		}
		else
		{
			if (v->callsign.size() > 0) CurCoTMsg->callsign = std::format("AIS{}", v->callsign);
			else CurCoTMsg->callsign = std::format("MSSI-{}", v->mmsi);
		}

		
		KnownVessel *kv = AIS_PARSER::FindKnownVesselByMMSI(v->mmsi);
		if (nullptr != kv) //it's a known vessel so use that known symbol code
		{
			CurCoTMsg->msg_type = kv->symbol;
			if (b_useKnownVessleCallsign) CurCoTMsg->callsign = kv->callsign;
			else CurCoTMsg->callsign = kv->name;
		}
		else  //determine symbol code based on AIS data (and hostility from MID data)
		{
			/*
			AIS Ship type - first digit
			1 = Reserved
			?2 = Wing In Ground
			?3 = Special Category
			?4 = High - Speed Craft
			?5 = Special Category
			?6 = Passenger
			?7 = Cargo
			?8 = Tanker
			?9 = Other
			ATAK icons ares something like  b-m-p-c-cp  for a circle (green)
			*/
			char hostility = GetHostilityFromMarineID(v->mmsi);  //checks country code against internal list of countries that are hostile (Russia, China for testing)
			CurCoTMsg->msg_type = std::format("a-{}-S-X-M",hostility);
		}
		

		CurCoTMsg->includeDetail = true;
		std::stringstream remarks;
		remarks << "<remarks>";
		if (CurCoTMsg->includeContact) remarks << "Shipname: " << v->callsign;
		if (name.size() > 0) remarks << " AIS Name: " << name;
		remarks << " Country: " << v->CountryFromMIDCode;
		remarks << " Type: " << std::to_string(v->type_and_cargo);
		remarks << " MMSI: " << std::to_string(v->mmsi);
		remarks << "</remarks>";
		CurCoTMsg->xmlDetail = remarks.str();

		CurCoTMsg->AssembleCoTPbufEvent();
		return CurCoTMsg;

		//std::string retVal = COTSENDER::SendCoTMsg(CurCoTMsg);
	}

	inline bg_TakMessage* AtoNToCoTMessage(AISVessel * v)
	{
		if (false == b_showAtoN) return nullptr;
		if (false == v->isValidAIS21) return nullptr;

		if ((0 == v->EntityLat) || (0 == v->EntityLng))
		{
			MsgFailCounts[21]++;
			return nullptr;  //Do not send COT with invalid positions
			//v->lat_deg = 41;
			//v->lng_deg = -64;
		}

		bg_TakMessage* CurCoTMsg = new bg_TakMessage();
		CurCoTMsg->IncludeTakControl = true;

		CurCoTMsg->d_lat =  v->EntityLat;
		CurCoTMsg->d_lon =  v->EntityLng;
		CurCoTMsg->d_hae = 0;
		if (0 == v->position_accuracy) //low
		{
			CurCoTMsg->d_ce = 100;
			CurCoTMsg->d_le = 100;
		}
		else //high 
		{
			CurCoTMsg->d_ce = 10;
			CurCoTMsg->d_le = 10;
		}
		

		CurCoTMsg->tm_validTimeInSeconds = 86400; //one day

		CurCoTMsg->UID = std::format("MMSI-{}", v->mmsi);

		CurCoTMsg->_how = getFixType(v->fix_type);

		CurCoTMsg->includeContact = true;
		std::string name{};
		
		//AidToNavigation* v2 = (AidToNavigation*)FindVesselByMMSI(v->mmsi);
		
		if (v->name.size()==0)
		{
			CurCoTMsg->callsign = v->name;
		}
		else 
			CurCoTMsg->callsign = std::format("AtoN {}", v->mmsi);

		//Symbol should be based on MMSI. See https://e-navigation.canada.ca/topics/aids/docs/ais-aton/what-is
		//Canadian AtoN MMSI are like: 99316xxxx
		//Physical	99MID1xxx	0	GPS
		//Synthetic	99MID1xxx	0	Surveyed
		//Virtual	99MID6xxx	1	Surveyed
		CurCoTMsg->msg_type = std::string("a-n-S-N");

		CurCoTMsg->includeDetail = true;
		std::stringstream remarks;
		remarks << "<remarks>";
		if (CurCoTMsg->includeContact)
		{
			if (v->extendedName.size() > 0) remarks << "Ext Name: " << v->extendedName;
			if (name.size() > 0) remarks << " AIS Name: " << name;
			remarks << " NavType: " << NAVAID_TYPE[v->NavType];
			if (v->virtualflag) remarks << " Virtual AidToNav";
			else remarks << " Real AidToNav";
			remarks << " MMSI: " << std::to_string(v->mmsi);
			remarks << "</remarks>";
			CurCoTMsg->xmlDetail = remarks.str();
		}


		CurCoTMsg->AssembleCoTPbufEvent();
		return CurCoTMsg;
		//std::string retVal = COTSENDER::SendCoTMsg(CurCoTMsg);
	}

	inline bg_TakMessage* SARAircraftToCoTMessage(AISVessel* v)
	{
		if (false == v->isValidAIS9)  return nullptr;

		bg_TakMessage *CurCoTMsg = new bg_TakMessage();
		CurCoTMsg->IncludeTakControl = true;

		CurCoTMsg->d_lat = v->EntityLat;
		CurCoTMsg->d_lon = v->EntityLng;
		CurCoTMsg->d_hae = 0;
		if (0 == v->position_accuracy) //low
		{
			CurCoTMsg->d_ce = 100;
			CurCoTMsg->d_le = 100;
		}
		else //high 
		{
			CurCoTMsg->d_ce = 10;
			CurCoTMsg->d_le = 10;
		}


		CurCoTMsg->tm_validTimeInSeconds = 90;

		CurCoTMsg->UID = std::format("MMSI-{}", v->mmsi);
		CurCoTMsg->_how = getFixType(v->fix_type);

		CurCoTMsg->course = v->cog;

		//AIS Speed over ground : 0.1 - knot(0.19 km / h) resolution from
		//                   0 to 102 knots(189 km / h)
		// COT Speed is meters / second
		// Pre - computed constant : 0.1 / 1.944 = 0.05144
		CurCoTMsg->speed = v->sog * 0.05144;


		CurCoTMsg->includeContact = true;
		if (v->callsign.size() > 0) CurCoTMsg->callsign = std::format("AIS{}", v->callsign);
		else CurCoTMsg->callsign = std::format("MSSI-{}", v->mmsi);
		char hostility = GetHostilityFromMarineID(v->mmsi);  //checks country code against internal list of countries that are hostile (Russia, China for testing)
		CurCoTMsg->msg_type = std::format("a-{}-A-M-F", hostility);

		//CurCoTMsg->msg_type = "a-f-A-M-F-H"; //CSAR Fixed Wing
		//CurCoTMsg->msg_type = "a-f-A-M-F-Q-H"; //CSAR Drone
		//CurCoTMsg->msg_type = "a-f-A-M-H-H"; //CSAR Helo
		//CurCoTMsg->msg_type = "a-f-A-M-F"; //SAR FixedSing



		CurCoTMsg->includeDetail = true;
		std::stringstream remarks;
		remarks << "<remarks>";
		remarks << "Call Sign: " << v->callsign;
		remarks << " Country: " << v->CountryFromMIDCode;
		remarks << " MMSI: " << std::to_string(v->mmsi);
		remarks << "</remarks>";
		CurCoTMsg->xmlDetail = remarks.str();

		CurCoTMsg->AssembleCoTPbufEvent();
		return CurCoTMsg;

		//std::string retVal = COTSENDER::SendCoTMsg(CurCoTMsg);
		//wxLogMessage(retVal.c_str());
	}



}

