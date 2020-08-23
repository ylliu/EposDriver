#include "EposDriver.h"
#include "Definitions.h"
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/time.h>
#include <vector>


#ifndef MMC_MAX_LOG_MSG_SIZE
#define MMC_MAX_LOG_MSG_SIZE 512
#endif

typedef void* HANDLE;
typedef int BOOL;

EposDriver::EposDriver(string usb_port_name, unsigned short node_id)
{
	g_portName = usb_port_name;
	g_usNodeId = node_id;
}

int EposDriver::InitDevice()
{
	int lResult = MMC_FAILED;
	if ((lResult = OpenDevice()) != MMC_SUCCESS)
	{
		LogError("OpenDevice", lResult, *p_pErrorCode);
		return lResult;
	}

	if ((lResult = CheckFaultState(p_pErrorCode)) != MMC_SUCCESS)
	{
		LogError("PrepareDemo", lResult, *p_pErrorCode);
		return lResult;
	}
	return lResult;
}

int EposDriver::DemoProfilePositionMode(long target_position)
{
	int lResult = MMC_SUCCESS;
	stringstream msg;

	msg << "set profile position mode, node = " << g_usNodeId;
	LogInfo(msg.str());

	if (VCS_ActivateProfilePositionMode(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
	{
		LogError("VCS_ActivateProfilePositionMode", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
		return lResult;
	}
	
	msg << "move to position = " << target_position << ", node = " << g_usNodeId;
	LogInfo(msg.str());

	if (VCS_MoveToPosition(g_pKeyHandle, g_usNodeId, target_position, 0, 1, p_pErrorCode) == 0)
	{
		LogError("VCS_MoveToPosition", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
	}
	sleep(1);

	if (lResult == MMC_SUCCESS)
	{
		LogInfo("halt position movement");

		if (VCS_HaltPositionMovement(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
		{
			LogError("VCS_HaltPositionMovement", lResult, *p_pErrorCode);
			lResult = MMC_FAILED;
		}
	}
	return lResult;
}

int EposDriver::DemoProfileVelocityMode(long target_velocity)
{
	int lResult = MMC_SUCCESS;
	stringstream msg;

	msg << "set profile velocity mode, node = " << g_usNodeId;

	LogInfo(msg.str());

	if (VCS_ActivateProfileVelocityMode(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
	{
		LogError("VCS_ActivateProfileVelocityMode", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
	}
	else
	{
		stringstream msg;
		msg << "move with target velocity = " << target_velocity << " rpm, node = " << g_usNodeId;
		LogInfo(msg.str());

		if (VCS_MoveWithVelocity(g_pKeyHandle, g_usNodeId, target_velocity, p_pErrorCode) == 0)
		{
			lResult = MMC_FAILED;
			LogError("VCS_MoveWithVelocity", lResult, *p_pErrorCode);
		}

	}
	return lResult;
}

int EposDriver::DemoStopVelocityMode()
{
	int lResult = MMC_SUCCESS;
	LogInfo("halt velocity movement");

	if (VCS_HaltVelocityMovement(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
	{
		lResult = MMC_FAILED;
		LogError("VCS_HaltVelocityMovement", lResult, *p_pErrorCode);
	}
	return lResult;
}

int EposDriver::DemoCurrentMode(int max_speed,int current)
{
	int lResult = MMC_SUCCESS;
	stringstream msg;

	msg << "set current mode, node = " << g_usNodeId;

	LogInfo(msg.str());
	if (VCS_ActivateCurrentMode(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
	{
		LogError("VCS_ActivateProfileCurrentMode", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
	}
	else
	{
		unsigned int written_byte = 0;
		if (0 == VCS_SetObject(g_pKeyHandle, g_usNodeId, 0x6410, 0x04, &max_speed, 1, &written_byte, p_pErrorCode))
		{
			std::cout << "set max_speed failed." << g_usNodeId << std::endl;
			return false;
		}
		std::cout << "set max_speed success,max_speed:" << max_speed << "written_byte:" << written_byte << std::endl;
		if (0 == VCS_SetEnableState(g_pKeyHandle, g_usNodeId, p_pErrorCode)){
			std::cout << "set enable failed." << g_usNodeId << std::endl;
			return false;
		}
		if (0 == VCS_SetCurrentMust(g_pKeyHandle, g_usNodeId, current, p_pErrorCode))
		{
			LogError("VCS_SetCurrentMust", lResult, *p_pErrorCode);
			lResult = MMC_FAILED;
		}
	}

	return lResult;
}

int EposDriver::DemoStopCurrentMode()
{
	int lResult = MMC_SUCCESS;

	if (0 == VCS_SetDisableState(g_pKeyHandle, g_usNodeId, p_pErrorCode)){
		LogError("VCS_SetDisableState", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
	}
	return lResult;
}

int EposDriver::GetAnalogData(int input_number, unsigned short& analog_value)
{
	int lResult = MMC_SUCCESS;
	if (0 == VCS_GetAnalogInput(g_pKeyHandle, g_usNodeId, input_number,&analog_value,p_pErrorCode)){
		LogError("GetAnalogData", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
	}
	return lResult;

}

int EposDriver::GetDigitalInput(int input_number, unsigned short& digital_value)
{
	int lResult = MMC_SUCCESS;
	if (0 == VCS_GetAllDigitalInputs(g_pKeyHandle, g_usNodeId, &digital_value, p_pErrorCode)){
		LogError("GetDigitalData", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
	}
	digital_value = digital_value & (0x0001 << input_number);
	return lResult;
}

int EposDriver::OpenDevice()
{
	int lResult = MMC_FAILED;

	char* pDeviceName = new char[255];
	char* pProtocolStackName = new char[255];
	char* pInterfaceName = new char[255];
	char* pPortName = new char[255];

	strcpy(pDeviceName, g_deviceName.c_str());
	strcpy(pProtocolStackName, g_protocolStackName.c_str());
	strcpy(pInterfaceName, g_interfaceName.c_str());
	strcpy(pPortName, g_portName.c_str());

	LogInfo("Open device...");
	g_pKeyHandle = VCS_OpenDevice(pDeviceName, pProtocolStackName, pInterfaceName, pPortName, p_pErrorCode);

	if (g_pKeyHandle != 0 && *p_pErrorCode == 0)
	{
		unsigned int lBaudrate = 0;
		unsigned int lTimeout = 0;

		if (VCS_GetProtocolStackSettings(g_pKeyHandle, &lBaudrate, &lTimeout, p_pErrorCode) != 0)
		{
			if (VCS_SetProtocolStackSettings(g_pKeyHandle, g_baudrate, lTimeout, p_pErrorCode) != 0)
			{
				if (VCS_GetProtocolStackSettings(g_pKeyHandle, &lBaudrate, &lTimeout, p_pErrorCode) != 0)
				{
					if (g_baudrate == (int)lBaudrate)
					{
						lResult = MMC_SUCCESS;

					}
				}
			}
		}
	}
	else
	{
		g_pKeyHandle = 0;
	}
	delete[]pDeviceName;
	delete[]pProtocolStackName;
	delete[]pInterfaceName;
	delete[]pPortName;

	return lResult;
}

int EposDriver::CheckFaultState(unsigned int* p_pErrorCode)
{
	int lResult = MMC_SUCCESS;
	BOOL oIsFault = 0;

	if (VCS_GetFaultState(g_pKeyHandle, g_usNodeId, &oIsFault, p_pErrorCode) == 0)
	{
		LogError("VCS_GetFaultState", lResult, *p_pErrorCode);
		lResult = MMC_FAILED;
	}

	if (lResult == 0)
	{
		if (oIsFault)
		{
			stringstream msg;
			msg << "clear fault, node = '" << g_usNodeId << "'";
			LogInfo(msg.str());

			if (VCS_ClearFault(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
			{
				LogError("VCS_ClearFault", lResult, *p_pErrorCode);
				lResult = MMC_FAILED;
			}
		}

		if (lResult == 0)
		{
			BOOL oIsEnabled = 0;

			if (VCS_GetEnableState(g_pKeyHandle, g_usNodeId, &oIsEnabled, p_pErrorCode) == 0)
			{
				LogError("VCS_GetEnableState", lResult, *p_pErrorCode);
				lResult = MMC_FAILED;
			}

			if (lResult == 0)
			{
				if (!oIsEnabled)
				{
					if (VCS_SetEnableState(g_pKeyHandle, g_usNodeId, p_pErrorCode) == 0)
					{
						LogError("VCS_SetEnableState", lResult, *p_pErrorCode);
						lResult = MMC_FAILED;
					}
				}
			}
		}
	}
	return lResult;

}

void EposDriver::LogError(string functionName, int p_lResult, unsigned int p_ulErrorCode)
{
	cerr << g_programName << ": " << functionName << " failed (result=" << p_lResult << ", errorCode=0x" << std::hex << p_ulErrorCode << ")" << endl;
}

void EposDriver::LogInfo(string message)
{
	cout << message << endl;
}

void EposDriver::PrintSettings()
{
	stringstream msg;

	msg << "default settings:" << endl;
	msg << "node id             = " << g_usNodeId << endl;
	msg << "device name         = '" << g_deviceName << "'" << endl;
	msg << "protocal stack name = '" << g_protocolStackName << "'" << endl;
	msg << "interface name      = '" << g_interfaceName << "'" << endl;
	msg << "port name           = '" << g_portName << "'" << endl;
	msg << "baudrate            = " << g_baudrate;

	LogInfo(msg.str());

	SeparatorLine();
}

void EposDriver::SeparatorLine()
{
	const int lineLength = 65;
	for (int i = 0; i < lineLength; i++)
	{
		cout << "-";
	}
	cout << endl;
}

