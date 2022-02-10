#pragma once
#include <Arduino.h>
#include "PinNumbers.h"

typedef uint8_t lState;

class LampOutput 
{
private:
    LampOutput()
    {
        init();
    }
public:
    static LampOutput& getSingleton()
    {
        static LampOutput lOutput;
        return lOutput;
    }

    const static size_t number_of_lamps = 3;
    
private:
    byte lamps[number_of_lamps] = {
        PinNumbers::LAMP_01, 
        PinNumbers::LAMP_02, 
        PinNumbers::LAMP_03
    };
    
    public:
    void init()
    {
        for(int i = 0; i < number_of_lamps; i++)
        {
            pinMode(lamps[i], OUTPUT);
        }
    }
    
    void output(int number, bool state)
    {
        digitalWrite(lamps[number], state);
    }

    void output(const lState* states)
    {
        for(int i = 0; i < number_of_lamps; i++)
        {
            digitalWrite(lamps[i], states[i]);
        }
    }
};