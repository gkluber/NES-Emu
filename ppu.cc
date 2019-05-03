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
	uint8_t PPUCTRL;
	uint8_t PPUMASK;
	uint8_t PPUSTATUS;
	uint8_t OAMADDR;
	uint8_t OAMDATA;
	// TODO scroll and PPUaddr latches
	uint8_t PPUSCROLL;
	uint8_t PPUADDR;
	uint8_t PPUDATA;
	uint8_t OAMDMA;
	 
	bool odd_frame;
		
	uint8_t mem[16384];	
	uint8_t nametables[4096];
	uint8_t secOAM[32];

	inline void resetSecondaryOAM()
	{
		for (int i = 0; i < 32; i++)
			secOAM[i] = 0xff;
	}
		
	void power()
	{
		PPUCTRL = 0;
		PPUMASK = 0;
		PPUSTATUS = 0b10100000;
		OAMADDR = 0;
		PPUSCROLL = 0;
		PPUADDR = 0;
		PPUDATA = 0;
		odd_frame = false;
	}
	
	void reset()
	{
		PPUCTRL = 0;
		PPUMASK = 0;
		PPUSTATUS &= 0b10000000;
		PPUSCROLL = 0;
		PPUDATA = 0;
		odd_frame = false;
	}
}
