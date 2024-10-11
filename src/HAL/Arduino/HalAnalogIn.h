#pragma once

#include <Arduino.h>
#include "../HalAnalogInBase.h"

class HalAnalogIn : public HalAnalogInBase
{
private:
	int _Pin;

public:
	HalAnalogIn(int pin)
	{
		_Pin = pin;
	}

protected:
	virtual void EnableImplement()
	{
#if defined ARDUINO_ARCH_STM32F4 || defined ARDUINO_ARCH_STM32
		pinMode(_Pin, INPUT_ANALOG);
#endif // ARDUINO_ARCH_STM32F4 || ARDUINO_ARCH_STM32

#if defined ARDUINO_ARCH_NRF52
		analogReadResolution(14);
#endif // ARDUINO_ARCH_NRF52
	}

	virtual float ReadImplement()
	{
#if defined ARDUINO_ARCH_NRF52
		return (float)analogRead(_Pin) / 16383 * 0.6f * 6 / 3.3f;
#else
		return (float)analogRead(_Pin) / 1023;
#endif
	}

};
