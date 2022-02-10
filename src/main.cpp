
#include <Arduino.h>
#include <WiFi.h>

#include "EEPROMSynch.h"


#define BAUD_RATE 9600

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

char ssid[] = "FRITZ!Box 6591 Cable RJ";
char password[] = "460653225093939888142020";


#include "HardwareInOutput/PinNumbers.h"
#include "Display/DisplayManager.h"

// #define DISP_CLK 22
// #define DISP_DIO 23
DisplayManager displayManager;

#include <DNSServer.h>

#include "EEPROMSynch.h"

#include "EEPROMSynch.h"
#include "MyWebServer.h"
MyWebServer myWebServer;

#include "HardwareInOutput/rcswitch_values.h"

MyTimer isr_timer(300), displayRefreshRate(500);

void IRAM_ATTR buttonPin_isr()
{
  if(!isr_timer.isReady())
  	return;
  displayManager.toggleMode();
}

char getLightIntensity()
{
	return 0;
}

void* rc_serial_com_routine(void* p)
{
	while(1)
	{
		// put your main code here, to run repeatedly:
		displayManager.update();

		EEPROM.commit();

		Serial.println("Re-send values via RC");
		for(int i = 0; i < LampOutput::number_of_lamps; i++)
		{
			EEPROMSynch::instance.sendLampStateToLamp(i);
		}
		Serial.println();
		delay(10);
	}
}

#include <pthread.h>
pthread_t rc_serial_com_thread;

void setup()
 {
	// put your setup code here, to run once:
	Serial.begin(BAUD_RATE);
	
	EEPROMSynch::instance.init_from_EEPROM();

	pinMode(PinNumbers::RC_SENDER_PIN, OUTPUT);
	RC::sender.enableTransmit(PinNumbers::RC_SENDER_PIN);

	Serial.println(ssid);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.printf("Cant connect with status=%d\n", WiFi.status());
	}
	Serial.println("WiFi connected.");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	myWebServer.initialize();
	
	Serial.println("Initializing Interupt...");

	pinMode(PinNumbers::BUTTON_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(PinNumbers::BUTTON_PIN), buttonPin_isr, FALLING);

	displayManager = DisplayManager(PinNumbers::DISP_CLK, PinNumbers::DISP_DIO, WiFi.localIP().toString(), displayRefreshRate);
	pthread_create(&rc_serial_com_thread, NULL, &rc_serial_com_routine, NULL);
}

void loop() {
	// Webserver handlers are supposed to run on main-thread
}