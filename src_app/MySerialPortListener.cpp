

//#include <iostream>
#include <sstream> // Required for std::istringstream
#include <string>
#include <format>

//#include "wx/log.h"

//void parseNMEA(std::string nmea);
void ProcessNMEALine(std::string nmea);


//from https://github.com/itas109/CSerialPort


#ifdef _WIN32
#include <windows.h>
#define imsleep(microsecond) Sleep(microsecond) // ms
#else
#include <unistd.h>
#define imsleep(microsecond) usleep(1000 * microsecond) // ms
#endif

//winmm.lib is a static link library in Windows that provides access to the Windows Multimedia API(WinMM).
#pragma comment(lib, "winmm.lib")

#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"

#include "MySerialPortListener.h"

using namespace itas109;


static std::string st_FindCOMPorts();
static std::string st_ComPort{};
static int st_BaudRate = 9600;
static bool restart = false;

void ParseNMEAFromSerial(std::string line)
{
    //wxLogMessage("In ParseNMEAFromSerial: %s", line.c_str());
    ProcessNMEALine(line);
}

class MySerialPortListener : public CSerialPortListener, public CSerialPortHotPlugListener
{
public:
    MySerialPortListener(CSerialPort* sp) : p_sp(sp) {};

    void onReadEvent(const char* portName, unsigned int readBufferLen)
    {
        if (readBufferLen > 0)
        {
            char* data = new char[readBufferLen + 1]; // add 1 for the terminating Zero
            if (data)
            {
                int recLen = p_sp->readData(data, readBufferLen);

                if (recLen > 0)
                {
                    data[recLen] = '\0';  //force terminal of the buffer for potential C style string handling

                    //TO DO. Preprocessing of data received from Serial Port
                    //change this to a more general algorithm:
                    // 1.   The 4800 GPS pucks send less than one NMEA sentence per ReadEvent. They need to be aggregated
                    // 2.   The uBlox sends multiple NMEA senetences per ReadEvent. These have to be broken up into sentences

                    //break up multiple lines into single lines for case 1 above
                    std::string sData(data);
                    std::istringstream iss(sData);
                    std::string line;

                    while (std::getline(iss, line))  // Read line by line until end of stream
                    {
                        ParseNMEAFromSerial(line);
                    }
                }
                delete[] data;
                data = NULL;
            }
        }
    };

    void onHotPlugEvent(const char* portName, int isAdd)
    {
        if (1 == isAdd)
        {
            //wxLogMessage("onHotPlugEvent: Found COM Port %s", portName);
        }

        else
        {
            //wxLogMessage("Lost COM Port %s", st_ComPort.c_str());
            restart = true;
        }
        //wxLogMessage("Hot Plug: portName: %s, %s: %d\n", portName, isAdd ? "discovered" : "lost");

        //FindCOMPorts();
    }

private:
    CSerialPort* p_sp;
};


std::vector<SerialPortInfo> m_availablePortsList;

//StaticUse Comm port is used for Hot Plug events
CSerialPort sp_StaticUse;  //there is a single serial port?
MySerialPortListener listenerStaticUse(&sp_StaticUse);

//sp_NMEA is used for reading (and sending) data
CSerialPort sp_NMEA;
MySerialPortListener listenerNMEA(&sp_NMEA);



static std::string st_FindCOMPorts()
{
    std::stringstream retVal{};

    m_availablePortsList.clear();
    m_availablePortsList = CSerialPortInfo::availablePortInfos();
    int availablePortCount = (int)m_availablePortsList.size();

    // No connect for read
    //sp_StaticUse.connectReadEvent(&listenerStaticUse);
    // connect for hot plug
    sp_StaticUse.connectHotPlugEvent(&listenerStaticUse);

    // connect for read
    sp_NMEA.connectReadEvent(&listenerNMEA);
    // No connect for hot plug
    //sp_NMEA.connectHotPlugEvent(&listenerNMEA);

    retVal << "Available COM Ports:";
    for (int i = 1; i <= availablePortCount; ++i)
    {
        //SerialPortInfo serialPortInfo = m_availablePortsList[i - 1]; //Generates Message: A sub-expression may overflow before being assigned to a wider type
        // which suggests the below change
        SerialPortInfo serialPortInfo = m_availablePortsList[static_cast<std::vector<itas109::SerialPortInfo, std::allocator<itas109::SerialPortInfo>>::size_type>(i) - 1];
        retVal << std::format("{:d} - {:s} {:s} {:s}", i, serialPortInfo.portName, serialPortInfo.description, serialPortInfo.hardwareId);
    }
    if (m_availablePortsList.size() == 0) retVal << "No valid serial ports";
    return retVal.str();
}


std::string CloseCOMPort()
{
    sp_NMEA.close();
    return std::format("Close {:s} -> Code: {:d}, Message: {:s}", "DEFAULT", sp_NMEA.getLastError(), sp_NMEA.getLastErrorMsg());
}

std::string OpenCOMPort(std::string portName, int baudrate)
{
    std::stringstream retVal{};

    st_ComPort = portName; //keep a local copy for auto restart
    sp_NMEA.init(st_ComPort.c_str(),    // examples: windows:COM1 Linux:/dev/ttyS0
        baudrate,
        itas109::ParityNone,   
        itas109::DataBits8,    
        itas109::StopOne,      
        itas109::FlowNone,     
        4096                // read buffer size
    );


    sp_NMEA.setReadIntervalTimeout(50);         // read interval timeout 0ms  ///////THIS WAS 0 and didn't work well for loopback
    sp_NMEA.setByteReadBufferFullNotify(3276); // 4096*0.8 // buffer full notify

    sp_NMEA.open();

    retVal << std::format("Open {} -> Code: {}, Message: {}\r\n", portName, sp_NMEA.getLastError(), sp_NMEA.getLastErrorMsg());

    if (0 == sp_NMEA.getLastError())
    {
        retVal << std::format("Opened COM Port {}\r\n", portName);
        restart = false;
        return retVal.str();
    }
    else // Error could not open
    {
        retVal << std::format("Code: {}, Message: {}\r\n", sp_NMEA.getLastError(), sp_NMEA.getLastErrorMsg());
        return retVal.str();
    }

    return retVal.str();
}




void CALLBACK SerialTimerProc(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
    if (restart)
    {
        //wxLogMessage("Trying to restart %s", st_ComPort.c_str());
        OpenCOMPort(st_ComPort, st_BaudRate);
    }
}



static void StartSerialProcessTimer()
{
    UINT uPeriod = 1000;  //this is timer period in ms
    UINT uResolution = 1;
    DWORD dwUser = NULL;
    UINT fuEvent = TIME_PERIODIC; //You also choose TIME_ONESHOT;
    timeBeginPeriod(10); //wait 10ms to start

    MMRESULT FTimerID = timeSetEvent(uPeriod, uResolution, (LPTIMECALLBACK)&SerialTimerProc, dwUser, fuEvent);
    if (FTimerID == NULL)
    {
        //wxLogMessage("Failed to generate multimedia timer.\n");
    }
}




void InitializeSerial()
{
    StartSerialProcessTimer();
    st_FindCOMPorts();
}

void ShutDownSerial()
{
    if (sp_StaticUse.open()) CloseCOMPort();
    if (sp_NMEA.open()) CloseCOMPort();
}


std::string  UpdateSerialSummary()
{
    if (sp_NMEA.isOpen()) return std::format("COM Port {} is open", sp_NMEA.getPortName());
    else return std::string("No COM Port open");
}
