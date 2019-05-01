#pragma once

#include <cstdint>

namespace PPU
{	
	extern uint8_t ppuctrl;
	extern uint8_t ppumask;
	extern uint8_t ppustatus;
	extern uint8_t oamaddr;
	extern uint8_t oamdata;
	// TODO scroll and ppuaddr latches
	extern uint8_t ppuscroll;
	extern uint8_t ppuaddr;
	extern uint8_t ppudata;
	extern bool odd_frame;
	
	extern uint8_t mem[16384];
	
	void power();
	void reset();
}
