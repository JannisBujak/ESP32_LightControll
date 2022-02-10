#pragma once

#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

namespace RealTime {

	const static int TIMEZONE = +1;
	const static int SECONDS_P_HOUR = 3600;

	NTPClient init_time_client(WiFiUDP& ntpUDP)
	{
		NTPClient timeClient(ntpUDP);
		timeClient.begin();
		timeClient.setTimeOffset(TIMEZONE * SECONDS_P_HOUR);
		return timeClient;
	}

	static NTPClient& getTimeObj()
	{
		static WiFiUDP ntpUDP;
		static NTPClient timeClient = init_time_client(ntpUDP);
		
		while(!timeClient.update()) {
			timeClient.forceUpdate();
		}
		return timeClient;
	}
}


struct Counter{
  uint8_t counter;
  bool updated;
  
  Counter(uint8_t counter = 0, bool updated = false)
    : counter(counter), updated(updated)
  {}

  void increase()
  {
    counter++;
    updated = true;
  }

  bool wasUpdated()
  {
    return updated;
  }

  uint8_t read()
  {
    updated = false;
    return counter;
  }

};

struct MyTimer {
private:
    long cooldown, last_passed;

public:
    MyTimer()
        : cooldown(0), last_passed(0)
    {}

    MyTimer(MyTimer& mt)
        : cooldown(mt.cooldown), last_passed(mt.last_passed)
    {}
    
    MyTimer(long cooldown)
        : cooldown(cooldown), last_passed(0)
    {}

    bool isReady()
    {
        long current = millis();
        bool result = ((current - last_passed) > cooldown);
        if(result){
            last_passed = current;
        }
        return result;
    }

};

struct TimedCounter : public Counter, public MyTimer
{

  TimedCounter(unsigned int cooldown)
    : Counter(), MyTimer(cooldown)
    {}

    void increase()
    {
      if(isReady())
      {
        ((Counter*)this)->increase();
      }    
    }
};
