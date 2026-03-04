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
#include <tchar.h>


extern int COT_MULTICAST_SEND_PORT;
extern char COT_MULTICAST_SEND_GROUP[20];

extern int NumCoTSent;
extern std::vector<MapEntity*> m_MapEntityList;


LPCWSTR filePath = L"c:/web_root/AISToCoT.ini";
std::string COM_Port = "COM1";
UINT baud = 9600;

int totalAISRxCount{};

int totalCoTTx{};

bool g_debug = true;



std::string LpwstrToString(LPWSTR szResult) {
	if (!szResult || szResult[0] == L'\0') return "";

	// 1. Calculate required buffer size
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, szResult, -1, NULL, 0, NULL, NULL);

	// 2. Allocate buffer
	std::string strTo(size_needed, 0);

	// 3. Convert
	WideCharToMultiByte(CP_UTF8, 0, szResult, -1, &strTo[0], size_needed, NULL, NULL);

	// Remove null terminator if it was added
	if (!strTo.empty() && strTo.back() == '\0') strTo.pop_back();

	return strTo;
}

void WCharToChar(char* dest, const wchar_t* source) {
	int i = 0;
	while (source[i] != L'\0') {
		// This cast will lose data for characters > 127
		dest[i] = static_cast<char>(source[i]);
		++i;
	}
	dest[i] = '\0'; // Null-terminate the string
}



void ProcessNMEALine(std::string nmea)
{
	if (nmea.size() < 20) return;
	std::string payload = AIS_PARSER::getAISPayloadFromNMEA(nmea);
	if (payload.size() < 1) return;

	AISObject* ao = AIS_PARSER::getAISObjectFromAISPayloadString(payload);
	if (nullptr == ao) return;
	totalAISRxCount++;

	bg_TakMessage* tm = NMEA_AIS2COT::AISObjectToCoTMessage(ao);
	if (nullptr != tm)
	{
		std::string retVal = COTSENDER::SendCoTMsg(*tm);
		totalCoTTx++;
	}

}


std::string GetIniStatus()
{
	std::stringstream ss{};
	ss << std::format("COM_Port: {}\r\n", COM_Port);
	ss << std::format("baud: {}\r\n", baud);
	ss << std::format("CoT_Port: {}\r\n", COT_MULTICAST_SEND_PORT);
	ss << std::format("CoT_IP: {}\r\n", COT_MULTICAST_SEND_GROUP);

	return ss.str();

}

void StartAISandCOT()
{
	
	const TCHAR* section = _T("CONFIGURATION");
	const TCHAR* key = _T("COM_Port");
	const TCHAR* defaultValue = _T("COM1");
	TCHAR returnedString[256]; // Buffer to receive the string
	DWORD bufferSize = sizeof(returnedString) / sizeof(TCHAR);

	GetPrivateProfileString(section,       // Section name (e.g., "[Settings]")
		key,           // Key name (e.g., "DatabasePath=")
		defaultValue,  // Default value if key is not found
		returnedString, // Buffer to fill with the value
		bufferSize,    // Size of the buffer (in characters)
		filePath);      // INI file path

	//COM_Port = LpwstrToString(returnedString);

	baud = GetPrivateProfileInt(L"CONFIGURATION", L"baud", 9600, filePath);


	const TCHAR* _key = _T("CoT_IP");
	const TCHAR* _defaultValue = _T("239.2.3.1");
	GetPrivateProfileString(section,       // Section name (e.g., "[Settings]")
		_key,           // Key name (e.g., "DatabasePath=")
		_defaultValue,  // Default value if key is not found
		returnedString, // Buffer to fill with the value
		bufferSize,    // Size of the buffer (in characters)
		filePath);      // INI file path

	//WCharToChar(COT_MULTICAST_SEND_GROUP, returnedString);


	COT_MULTICAST_SEND_PORT = GetPrivateProfileInt(L"CONFIGURATION", L"CoT_Port", 6969, filePath);

	




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
	retVal = OpenCOMPort(COM_Port, baud);
	printf(retVal.c_str());
}

void StopAISandCOT()
{
	std::string r = CloseCOMPort();
	ShutDownSerial();

	COTSENDER::StopCOTSender();
	closeandclean_winsock();
}



