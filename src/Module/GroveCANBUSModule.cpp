#include "GroveCANBUSModule.h"
#include <string.h>
#include <stdio.h>

bool GroveCANBUSModule::Init()
{
	// Set default speed to 500KBPS using AT commands
	if (!SetSpeed(SPEED_500KBPS))
		return false;
	
	_IsExist = true;
	return true;
}

bool GroveCANBUSModule::SetSpeed(SPEED speed)
{
	if (!EnterSettingMode())
		return false;
	
	char command[16];
	if (speed < 10)
		sprintf(command, "AT+C=0%d\r\n", (int)speed);
	else
		sprintf(command, "AT+C=%d\r\n", (int)speed);
	
	bool result = SendATCommand(command);
	ExitSettingMode();
	return result;
}

bool GroveCANBUSModule::SendMessage(const CANMessage& message)
{
	// Format: ID3 ID2 ID1 ID0 EXT RTR DTA0 DTA1 DTA2 DTA3 DTA4 DTA5 DTA6 DTA7
	uint8_t data[14] = {0};
	
	// Set CAN ID (4 bytes)
	data[0] = (message.id >> 24) & 0xFF;
	data[1] = (message.id >> 16) & 0xFF;
	data[2] = (message.id >> 8) & 0xFF;
	data[3] = message.id & 0xFF;
	
	// Set flags
	data[4] = message.isExtended ? 1 : 0;  // EXT flag
	data[5] = message.isRemote ? 1 : 0;    // RTR flag
	
	// Set data bytes (8 bytes)
	for (int i = 0; i < 8; i++)
	{
		if (i < message.length)
			data[6 + i] = message.data[i];
		else
			data[6 + i] = 0;
	}
	
	// Send binary data
	for (int i = 0; i < 14; i++)
	{
		_UART->Write(data[i]);
	}
	
	return true;
}

bool GroveCANBUSModule::ReceiveMessage(CANMessage& message)
{
	if (_UART->ReadAvailable() >= 12)  // Need at least 12 bytes for a message
	{
		uint8_t data[12];
		
		// Read 12 bytes
		for (int i = 0; i < 12; i++)
		{
			data[i] = _UART->Read();
		}
		
		// Parse message
		message.id = ((uint32_t)data[0] << 24) | 
		            ((uint32_t)data[1] << 16) | 
		            ((uint32_t)data[2] << 8) | 
		            data[3];
		
		message.isExtended = (data[4] != 0);
		message.isRemote = (data[5] != 0);
		message.length = 8;  // Always 8 bytes in this format
		
		// Copy data bytes
		for (int i = 0; i < 8; i++)
		{
			message.data[i] = data[4 + i];  // Data starts at byte 4 in received format
		}
		
		return true;
	}
	
	return false;
}

void GroveCANBUSModule::AttachMessageReceived(void (*callback)(const CANMessage& message))
{
	_MessageReceivedCallback = callback;
}

void GroveCANBUSModule::DoWork()
{
	CANMessage message;
	if (ReceiveMessage(message) && _MessageReceivedCallback)
	{
		_MessageReceivedCallback(message);
	}
}

bool GroveCANBUSModule::EnterSettingMode()
{
	_UART->Write('+');
	_UART->Write('+');
	_UART->Write('+');
	
	ClearBuffer();
	HalSystem::Delay(100);
	return true;
}

bool GroveCANBUSModule::ExitSettingMode()
{
	return SendATCommand("AT+Q\r\n");
}

void GroveCANBUSModule::ClearBuffer()
{
	unsigned long startTime = HalSystem::ClockMs();
	while (HalSystem::ClockMs() - startTime < 50)
	{
		while (_UART->ReadAvailable() > 0)
		{
			_UART->Read();
			startTime = HalSystem::ClockMs();
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
					
					// Check for "OK" response
					if (responseIndex >= 2 && 
					    response[responseIndex-2] == 'O' && 
					    response[responseIndex-1] == 'K')
					{
						ClearBuffer();
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
