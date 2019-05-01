#include "ppu.h"

#include <cstdio>
#include <iostream>

using namespace PPU;

namespace PPU
{
	// PPU internal registers
	uint16_t v; // Current VRAM address
	uint16_t t; // Temporary VRAM address
	uint16_t x; // Fine X scroll
	uint16_t w; // First or second write toggle

	// Exposed registers	
	uint8_t ppuctrl;
	uint8_t ppumask;
	uint8_t ppustatus;
	uint8_t oamaddr;
	uint8_t oamdata;
	// TODO scroll and ppuaddr latches
	uint8_t ppuscroll;
	uint8_t ppuaddr;
	uint8_t ppudata;
	bool odd_frame;
	
	uint8_t mem[16384];
	
	
		
	void power()
	{
		ppuctrl = 0;
		ppumask = 0;
		ppustatus = 0b10100000;
		oamaddr = 0;
		ppuscroll = 0;
		ppuaddr = 0;
		ppudata = 0;
		odd_frame = false;
	}
	
	void reset()
	{
		ppuctrl = 0;
		ppumask = 0;
		ppustatus &= 0b10000000;
		ppuscroll = 0;
		ppudata = 0;
		odd_frame = false;
	}
}
