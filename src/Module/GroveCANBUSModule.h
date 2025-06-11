//GROVE_NAME        "Grove - CAN BUS Module based on GD32E103"
//SKU               101020782
//WIKI_URL          https://wiki.seeedstudio.com/Grove-CAN_BUS_Module-GD32/

#pragma once

#include "Abstract/GroveModule2.h"
#include "../Connector/GroveConnectorUART.h"

struct CANMessage
{
	uint32_t id;			// CAN ID (11-bit or 29-bit)
	bool isExtended;		// Extended ID flag
	bool isRemote;			// Remote transmission request flag
	uint8_t length;			// Data length (0-8 bytes)
	uint8_t data[8];		// Data bytes
};

class GroveCANBUSModule : public GroveModule2
{
public:
	enum SPEED
	{
		SPEED_5KBPS = 1,
		SPEED_10KBPS = 2,
		SPEED_20KBPS = 3,
		SPEED_25KBPS = 4,
		SPEED_31K25BPS = 5,
		SPEED_33KBPS = 6,
		SPEED_40KBPS = 7,
		SPEED_50KBPS = 8,
		SPEED_80KBPS = 9,
		SPEED_83K3BPS = 10,
		SPEED_95KBPS = 11,
		SPEED_100KBPS = 12,
		SPEED_125KBPS = 13,
		SPEED_200KBPS = 14,
		SPEED_250KBPS = 15,
		SPEED_500KBPS = 16,
		SPEED_666KBPS = 17,
		SPEED_1000KBPS = 18
	};

private:
	HalUART* _UART;
	void (*_MessageReceivedCallback)(const CANMessage& message);

	bool SendATCommand(const char* command, const char* expectedResponse = "OK", int timeout = 1000);
	bool EnterSettingMode();
	bool ExitSettingMode();
	void ClearBuffer();

public:
	GroveCANBUSModule(GroveConnectorUART* connector)
	{
		_UART = &connector->UART;
		_MessageReceivedCallback = nullptr;
	}

	bool Init();
	bool SetSpeed(SPEED speed);
	bool SendMessage(const CANMessage& message);
	bool ReceiveMessage(CANMessage& message);
	void AttachMessageReceived(void (*callback)(const CANMessage& message));
	void DoWork();

};
