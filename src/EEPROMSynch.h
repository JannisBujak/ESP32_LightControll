#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "HardwareInOutput/LampOutput.h"

#include "HardwareInOutput/rcswitch_values.h"

struct EEPROMSynch {
public:
private:
    using DataArray = std::array<lState, LampOutput::number_of_lamps>;
    DataArray data;
    
public:
    void init_from_EEPROM()
    {
        EEPROM.begin(data.size());
        EEPROM.readBytes(0, (char*)&data, data.size());
    } 

    lState getLampState(int lamp_index)
    {
        return data[lamp_index];
    }

    const lState* getLampStates()
    {
        return data.data();
    }

    void sendLampStateToLamp(int lamp_index)
    {
        std::pair<uint32_t, uint32_t>& p = RC::channel_I[lamp_index];
        Serial.printf("L%d:\t%d\t", lamp_index, getLampState(lamp_index));
        RC::sender.send(getLampState(lamp_index) ? p.first : p.second, 32);
    }

    void setLampState(int lamp_index, lState new_state)
    {
        data[lamp_index] = new_state;
        EEPROM.writeBytes(lamp_index, (char*)&(data[lamp_index]), 1);

    }

    void printStates()
    {
        for(int i = 0; i < LampOutput::number_of_lamps; i++)
        {
            Serial.print(data[i]);
            Serial.print("\t");
        }
        Serial.println();
    }


public:
    static EEPROMSynch instance;
};



EEPROMSynch EEPROMSynch::instance;