#pragma once

#include <cstdint>

namespace PPU
{	
	extern uint8_t PPUCTRL;
	extern uint8_t PPUMASK;
	extern uint8_t PPUSTATUS;
	extern uint8_t OAMADDR;
	extern uint8_t OAMDATA;
	// TODO scroll and ppuaddr latches
	extern uint8_t PPUSCROLL;
	extern uint8_t PPUADDR;
	extern uint8_t PPUDATA;
	extern uint8_t OAMDMA; 
	extern bool odd_frame;
		
	void power();
	void reset();
}
