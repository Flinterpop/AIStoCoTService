// AISTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "mongoose.h"
#include <conio.h>  // Required for kbhit() and getch()

#include "BG_SocketBase.h"
#include "MySerialPortListener.h"

#include "MapEntity.h"
#include "AISParser.h"
#include "AIStoCoT.h"


#include "COTSender.h"
#include "bg_TakMessage.h"




bool g_debug = true;

extern int NumCoTSent;
extern std::vector<MapEntity*> m_MapEntityList;



void ProcessNMEALine(std::string nmea)
{
	if (nmea.size() < 20) return;
	std::string payload = AIS_PARSER::getAISPayloadFromNMEA(nmea);
	if (payload.size() < 1) return;

	AISObject* ao = AIS_PARSER::getAISObjectFromAISPayloadString(payload);
	if (nullptr == ao) return;

	bg_TakMessage* tm = NMEA_AIS2COT::AISObjectToCoTMessage(ao);
	if (nullptr != tm) std::string retVal = COTSENDER::SendCoTMsg(*tm);

}

void StartAISandCOT()
{
	initialise_winsock();
	//getNetworkAdapterInfo();

	LoadKnownVesselList();
	LoadMIDTable();

	std::string retVal = COTSENDER::StartCOTSender();

	
	
	//TEST
	std::vector<std::string> nmeaList;
	nmeaList.push_back("!AIVDM,1,1,,A,1Cu?etPjh0KT>H@I;dL1hVv00000,0*57");
	nmeaList.push_back("!AIVDM,2,1,0,A,5Cu?etP00000<L4`000<P58hEA@E@uLp0000000S>8OA;0jjf012AhV@,0*47");
	nmeaList.push_back("!AIVDM,2,2,0,A,000000000000000,2*24");
	for (std::string nmea : nmeaList) ProcessNMEALine(nmea);


	InitializeSerial();
	retVal = OpenCOMPort("COM1", 9600);
	printf(retVal.c_str());
}

void StopAISandCOT()
{
	std::string r = CloseCOMPort();
	ShutDownSerial();

	COTSENDER::StopCOTSender();
	closeandclean_winsock();
}



