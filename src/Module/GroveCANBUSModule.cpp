#include "GroveCANBUSModule.h"
#include <string.h>
#include <stdio.h>

bool GroveCANBUSModule::Init()
{
	// Reset the module to factory defaults
	if (!SendATCommand("AT+RST"))
		return false;
	
	// Wait for module to restart
	HalSystem::Delay(2000);
	
	// Enter configuration mode
	if (!SendATCommand("AT+CGAUGE=0"))
		return false;
	
	// Set default speed to 250KBPS
	if (!SetSpeed(SPEED_250KBPS))
		return false;
		
	// Start CAN communication
	if (!SendATCommand("AT+CGAUGE=1"))
		return false;
	
	_IsExist = true;
	return true;
}

bool GroveCANBUSModule::SetSpeed(SPEED speed)
{
	char command[32];
	sprintf(command, "AT+CANSPEED=%d", (int)speed);
	return SendATCommand(command);
}

bool GroveCANBUSModule::SendMessage(const CANMessage& message)
{
	char command[64];
	char dataStr[17] = {0}; // 8 bytes = 16 hex chars + null terminator
	
	// Convert data to hex string
	for (int i = 0; i < message.length && i < 8; i++)
	{
		sprintf(dataStr + i*2, "%02X", message.data[i]);
	}
	
	// Build AT command for sending CAN message
	// Format: AT+CANSEND=ID,EXTENDED,REMOTE,LENGTH,DATA
	sprintf(command, "AT+CANSEND=%08X,%d,%d,%d,%s", 
			message.id, 
			message.isExtended ? 1 : 0,
			message.isRemote ? 1 : 0,
			message.length,
			dataStr);
	
	return SendATCommand(command);
}

void GroveCANBUSModule::AttachMessageReceived(void (*callback)(const CANMessage& message))
{
	_MessageReceivedCallback = callback;
}

void GroveCANBUSModule::DoWork()
{
	// Check for incoming messages
	if (_UART->ReadAvailable() > 0)
	{
		static char buffer[128];
		static int bufferIndex = 0;
		
		while (_UART->ReadAvailable() > 0)
		{
			char c = _UART->Read();
			
			if (c == '\n' || c == '\r')
			{
				if (bufferIndex > 0)
				{
					buffer[bufferIndex] = '\0';
					
					// Parse CAN message if it's a received message
					if (strncmp(buffer, "+CANRECV:", 9) == 0)
					{
						CANMessage message;
						if (ParseCANMessage(buffer, message) && _MessageReceivedCallback)
						{
							_MessageReceivedCallback(message);
						}
					}
					
					bufferIndex = 0;
				}
			}
			else if (bufferIndex < sizeof(buffer) - 1)
			{
				buffer[bufferIndex++] = c;
			}
		}
	}
}

bool GroveCANBUSModule::SendATCommand(const char* command, const char* expectedResponse, int timeout)
{
	// Send command
	for (const char* p = command; *p; p++)
	{
		_UART->Write(*p);
	}
	_UART->Write('\r');
	_UART->Write('\n');
	
	// Wait for response
	unsigned long startTime = HalSystem::ClockMs();
	char response[64];
	int responseIndex = 0;
	
	while (HalSystem::ClockMs() - startTime < timeout)
	{
		if (_UART->ReadAvailable() > 0)
		{
			char c = _UART->Read();
			
			if (c == '\n' || c == '\r')
			{
				if (responseIndex > 0)
				{
					response[responseIndex] = '\0';
					
					// Check if response matches expected
					if (strstr(response, expectedResponse) != nullptr)
					{
						return true;
					}
					
					responseIndex = 0;
				}
			}
			else if (responseIndex < sizeof(response) - 1)
			{
				response[responseIndex++] = c;
			}
		}
		
		HalSystem::Delay(1);
	}
	
	return false;
}

bool GroveCANBUSModule::ParseCANMessage(const char* response, CANMessage& message)
{
	// Parse format: +CANRECV:ID,EXTENDED,REMOTE,LENGTH,DATA
	if (strncmp(response, "+CANRECV:", 9) != 0)
		return false;
	
	const char* data = response + 9;
	
	// Parse ID
	if (sscanf(data, "%08X", &message.id) != 1)
		return false;
	
	// Find next comma
	data = strchr(data, ',');
	if (!data) return false;
	data++;
	
	// Parse extended flag
	int extended;
	if (sscanf(data, "%d", &extended) != 1)
		return false;
	message.isExtended = (extended != 0);
	
	// Find next comma
	data = strchr(data, ',');
	if (!data) return false;
	data++;
	
	// Parse remote flag
	int remote;
	if (sscanf(data, "%d", &remote) != 1)
		return false;
	message.isRemote = (remote != 0);
	
	// Find next comma
	data = strchr(data, ',');
	if (!data) return false;
	data++;
	
	// Parse length
	int length;
	if (sscanf(data, "%d", &length) != 1)
		return false;
	message.length = (uint8_t)length;
	
	// Find next comma
	data = strchr(data, ',');
	if (!data) return false;
	data++;
	
	// Parse data bytes
	for (int i = 0; i < message.length && i < 8; i++)
	{
		unsigned int byte;
		if (sscanf(data + i*2, "%02X", &byte) != 1)
			return false;
		message.data[i] = (uint8_t)byte;
	}
	
	return true;
}
