
#include <Arduino.h>
#pragma once

#include <RCSwitch.h>

namespace RC {
	std::pair<uint32_t, uint32_t> channel_I[] = {
		{ 0x8E752400, 0x81752400 }, 
		{ 0xA6752400, 0xAE752400 }, 
		{ 0x96752400, 0x9E752400 }, 
		{ 0x93752400, 0xA3752400 }	
	};

	RCSwitch sender;
}



