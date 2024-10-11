#pragma once

#if defined ARDUINO_WIO_BG770A

#include <Wire.h>

class GroveBoard
{
public:
    GroveConnectorDIO Digital;
    GroveConnectorAnalogIn Analog;
    GroveConnectorI2C I2C;
    GroveConnectorUART UART;

public:
    GroveBoard(void) : Digital{D30, D31},
                       Analog{A4, A5},
                       I2C{&Wire},
                       UART{&Serial1}
    {
    }
};

#endif // ARDUINO_WIO_BG770A
