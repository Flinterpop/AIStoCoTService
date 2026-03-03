

std::string UpdateSerialSummary();

void InitializeSerial();
void ShutDownSerial();

std::string OpenCOMPort(std::string portName, int baudrate=9600);
std::string CloseCOMPort();

