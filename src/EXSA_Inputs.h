#pragma once
#include <stdint.h>
#include <Adafruit_MCP23X17.h>

class EXSA_Inputs
{
public:
    static void begin();

    static bool readDipHoraire();
    static bool readDipBooster();

    // Debug / extensions
    static bool readRaw(uint8_t pin);

private:
    static Adafruit_MCP23X17 mcp;
};
