#pragma once

#include <Arduino.h>
#include <TM1637Display.h>
#include "TimedCounter.h"

#include <NTPClient.h>


/*
		--A--
	F	|	|	B
		+-G-+
	E	|	|	C
		--D--
	using 0bXGFEDCBA

examples:
    XGFEDCBA
 0b 00111111 0
 0b 00000110 1
 0b 01011011 2
 0b 01001111 3
 0b 01100110 4
 0b 01101101 5
 0b 01111101 6
 0b 00000111 7
 0b	01111111 8
 0b	01101111 9
 0b	01110111 A
 0b	01111100 B
 0b	00111001 C
 0b	01000111 D
 0b	01111001 E
 0b	01110001 F

*/

class DisplayManager 
{
private:    
    enum class Mode
    {
        doNothing,
        Show_IP_Once,
        Show_IP,
        Show_Time
    };

    String modeToString(Mode mode)
    {
        switch(mode)
        {
            case Mode::Show_IP_Once:
                return "Show_IP_Once";
            case Mode::Show_IP:
                return "Show_IP";
            case Mode::Show_Time:
                return "Show_Time";
            case Mode::doNothing:
            default:
                return "doNothing";
        }
    }

private:
    int ip_index;
    String ip_string;
    
    TM1637Display* display;
    
    Mode mode;
  
    MyTimer myTimer;
    
    

private:
    static const size_t NUMBER_OF_SEGMENTS = 4;
    
public:
    DisplayManager()
        : ip_index(0), display(nullptr), mode(Mode::doNothing)
    {}
    
    DisplayManager(int pin_clk, int pin_dio, String ip_string, MyTimer timer)
        : ip_index(0),
        ip_string(ip_string), 
        display(new TM1637Display(pin_clk, pin_dio)), 
        mode(Mode::Show_IP_Once),
        myTimer(timer)    
    {
        display->setBrightness(0x80);
    }

private:
    uint8_t getCharToBitarray(char c) const
    {
        switch(c)
        {
            case 'A':
                return 0b01111100;
            case 'B':
                return 0b01110111;
            case 'C':
                return 0b00111001;
            case 'D':
                return 0b01000111;
            case 'E':
                return 0b01111001;
            case 'F':
                return 0b01110001;
            case 'I':
                return 0b00110000;
            case 'P':
                return 0b01110011;
            default:
                return 0;
        }
    }

    void setAccordingToFIrstFour(const char* str)
    {
    uint8_t segments[NUMBER_OF_SEGMENTS] = {};
    for(int i = 0; i < NUMBER_OF_SEGMENTS; i++)
    {
        if(i >= strlen(str) || !isDigit(str[i]))
            segments[i] = getCharToBitarray(str[i]);
        else
            segments[i] = display->encodeDigit(str[i] - '0');
    }
        display->setSegments(segments);
    }

    void setSegmentsForTactI(String str, int& i)
    {
        const static String segment_string = String("IP " + str + " ") ;
        
		if(i > segment_string.length())
			i %= segment_string.length();
		
		String current_string = segment_string.substring(i) + segment_string;
		Serial.println(current_string);
        setAccordingToFIrstFour(current_string.c_str());
    }

public:
    void toggleMode()
    {
		switch (mode)
        {
        case Mode::Show_IP:
        case Mode::Show_IP_Once:
            mode = Mode::Show_Time;
            break;
        case Mode::Show_Time:
            mode = Mode::Show_IP;
            break;
        default:
            break;
        }
    }

    void update()
    {
        NTPClient& cl = RealTime::getTimeObj();
        if(!myTimer.isReady())
            return;

        switch (mode)
        {
        case Mode::Show_IP_Once:
			if(ip_index >= ip_string.length())
			{
				toggleMode();
			}
        case Mode::Show_IP:
            {
                setSegmentsForTactI(ip_string, ip_index);
                ip_index++;
            }
            break;
        case Mode::Show_Time:
            ip_index = 0;
            display->showNumberDecEx(cl.getHours() * 100 + cl.getMinutes(), ~((uint8_t)0), true);
        default:
            break;
        }
    }
};
