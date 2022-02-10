#pragma once

// HTTP-Server stuff
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"

#include "EEPROMSynch.h"
#include <Arduino.h>
#include <stdlib.h>

const char PARAM_INPUT_1[] = "lamp_nr";
const char PARAM_INPUT_2[] = "state";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<!DOCTYPE html>
<html lang en>
    <head>
        <meta charset='UTF-8'>

		<meta name="viewport" content="width=device-width, initial-scale=1">
		<link rel="icon" href="data:,">

        <title>ESP Web Server</title>
<style>
select {
  -webkit-appearance: none;
  border-radius: 0px;
  border-width: 0px;
  margin-left: 10px;
}
body {
  background-color: black;
}

.beispiel {
  color: white;
  width: 15em;
  border: 1px solid #333;
  box-shadow: 8px 8px 5px #444;
  padding: 8px 12px;
  background-image: linear-gradient(180deg, #fff, #ddd 40%%, #ccc);
}

h1 {
  color: orange;
  margin-left: 10px;
}

p {
  color: orange;
  margin-left: 10px;
}

.button {
  text-align: center;
  color: white;

  background-color: orangered;
  
  opacity: 0.7;

  height: 50px;
  width: 15%%;
  margin-left: 10px;
}
.button:hover {
  opacity: 1; /* Fully shown on mouse-over */
}
.slidecontainer {
  width: 100%%; /* Width of the outside container */  
  float: right;
  
  padding-left: 5px;
  padding-right: 5px;
}
/* The slider itself */
.slider {
  -webkit-appearance: none;  /* Override default CSS styles */
  appearance: none;
  width: 100%%; /* Full-width */
  height: 40px;
  background: lightblue; /* Grey background */
  outline: none; /* Remove outline */
  opacity: 0.7; /* Set transparency (for mouse-over effects on hover) */
  -webkit-transition: .2s; /* 0.2 seconds transition on hover */
  transition: .2s;
}  

/* Mouse-over effects */
.slider:hover {
  opacity: 1; /* Fully shown on mouse-over */
}
/* The slider handle (use -webkit- (Chrome, Opera, Safari, Edge) and -moz- (Firefox) to override default look) */
.slider::-webkit-slider-thumb {
  -webkit-appearance: none; /* Override default look */
  appearance: none;
  width: 25px; /* Set a specific slider handle width */
  height: 50px; /* Slider handle height */
  background: orangered; /* Green background */
  border-width: 0px;
  cursor: pointer /* Cursor on hover */
}
.slider::-moz-range-thumb {
  width: 25px; /* Set a specific slider handle width */
  height: 50px; /* Slider handle height */
  background: orangered; /* Green background */
  border-width: 0px;
  cursor: pointer; /* Cursor on hover */
  
}

</style>
    </head>
    <body>
        <script>
            function setSliderTo(lamp_nr, value) 
            {    
                var slider = document.getElementById('span_light'+lamp_nr);
                slider.value = value;
                var chEvent = new Event('change');
                slider.dispatchEvent(chEvent)
            }
            function startUpdate(lamp_nr, value)
            {
                var xhr = new XMLHttpRequest();
				xhr.open("GET", "/update?lamp_nr="+lamp_nr+"&state="+value, true); 
                xhr.send();
            }
        </script>
        
        <h1>Licht einstellen</h1>
        
		%SLIDEMENTS%
    </body>
</html>
)rawliteral";

class MyWebServer
{
public:
	MyWebServer()
	{
		server = (new AsyncWebServer(80));
	}
private:
	DNSServer dnsServer;
	AsyncWebServer* server;

	const static String light_names[]; 

	static String outputState(int output)
	{
		int state = EEPROMSynch::instance.getLampState(output);
		Serial.printf("Setting lamp \"%s\"(%d) to %d\n", light_names[output].c_str(), output, state);
		if(state)
			return "checked=\"true\"";
		else
			return "";
	}

	static String open_div()
	{
		return "<div style='display:flex'; flex-direction: row; justify-content: left; align-items: center>";
	}

	static String create_sliderSettingBtn(int lamp_nr, int slider_value, String text)
	{
		return "<button class='button' onclick='setSliderTo(" + String(lamp_nr) + ", " + String(slider_value) + ")'>" + text + "</button>";
	}

	static String create_slider(int lamp_nr)
	{
		return "<span class='slidecontainer'>\
		<input type='range' min='0' max='10' value='" + String(EEPROMSynch::instance.getLampState(lamp_nr)) + "' class='slider' id='span_light"+String(lamp_nr)+"' onchange='startUpdate("+String(lamp_nr)+", this.value)'>\
		</span>";
	}

	static String close_div()
	{
		return "</div>";
	}

	static String processor(const String& var)
	{	
		if(var == "")
		{
			return String("%");
		}else if(var == "SLIDEMENTS"){
			String buttons = "";

			for(int i = 0; i < LampOutput::number_of_lamps; i++)
			{
				buttons += "<p>"+light_names[i]+"</p>"
				+ open_div()
				+ create_sliderSettingBtn(i, 0, "Immer aus")
				+ create_slider(i)
				+ create_sliderSettingBtn(i, 10, "Immer an")
				+ close_div();
			}
			return buttons;
		}
		return String();
	}

public:
  void initialize()
  {
	server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send_P(200, "text/html", index_html, processor);
	});

	// Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
	server->on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {

		String inputMessage1, inputMessage2;
		// GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
		if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
			inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
			inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
		}
		else {
			inputMessage1 = "No message sent";
			inputMessage2 = "No message sent";
		}
		Serial.printf("GPIO: %s - Set to: %s\n\n", inputMessage1.c_str(), inputMessage2.c_str());
		
		long long light_changed, state;
		light_changed = strtoll(inputMessage1.c_str(), nullptr, 10);
		state         = strtoll(inputMessage2.c_str(), nullptr, 10);

		EEPROMSynch::instance.setLampState(light_changed, state);
		
		LampOutput::getSingleton().output(EEPROMSynch::instance.getLampStates());
		EEPROMSynch::instance.sendLampStateToLamp(light_changed);
		
		request->send(200, "text/plain", "OK");
	});

	// Start server
  	server->begin();
  }
};  

const String MyWebServer::light_names[] =
	{
		"Schreibtisch",
		"Nachttisch",
		"Unbelegt"
	};