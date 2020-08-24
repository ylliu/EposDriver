#ifndef EPOS_DRIVER_2020_08_23_H_ 
#define EPOS_DRIVER_2020_08_23_H_
#include <iostream>
#include <string>
#ifndef MMC_SUCCESS
#define MMC_SUCCESS 0
#endif

#ifndef MMC_FAILED
#define MMC_FAILED 1
#endif
using namespace std;
class EposDriver
{
public:
	EposDriver(string usb_port_name, unsigned short node_id);
	~EposDriver(){};
public:
	int InitDevice();
	int DemoProfilePositionMode(long target_position);
	int DemoProfileVelocityMode(long target_velocity);
	int DemoStopVelocityMode();
	int DemoCurrentMode(int max_speed,int current);
	int DemoStopCurrentMode();
	//Analog return value is int type you should divide by a factor
	int GetAnalogData(int input_number, unsigned short& analog_value); 
	//input_number form zero
	int GetDigitalInput(int input_number, unsigned short& digital_value); 
protected:
	
private:
	int OpenDevice();
	int CheckFaultState(unsigned int* p_pErrorCode);
	void LogError(string functionName, int p_lResult, unsigned int p_ulErrorCode);
	void LogInfo(string message);
	void PrintSettings();
	void SeparatorLine();
private:

	const string g_deviceName = "EPOS2";   //default setting
	const string g_protocolStackName = "MAXON SERIAL V2";
	const string g_interfaceName = "USB";
	const int g_baudrate = 1000000;
	string g_portName = "";  //from OpenDevice 
	const string g_programName = "EposDemoDriver";
	unsigned int p_pErrorCode;
	void* g_pKeyHandle = 0;
	unsigned short g_usNodeId = 1;
};
#endif
