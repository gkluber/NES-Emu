#include "ppu.h"

using namespace PPU;

SDL_Window *window;		
SDL_Surface *screen;
SDL_Renderer *renderer;		

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
	uint64_t chrom[1024];
	uint8_t OAM[256];
	uint8_t secOAM[32];

	uint8_t** readBackgroundPalette(int x, int y) {
		// find attribute in memory for the segment of the window at coordinates x,y
		// this will change, 960 refers to last address of the nametable it follows, x and y offset correct
		int attributeIndex = 960 + x / 32 + (y / 32 * 8);
		// find which of the 4 4x4pixel tiles is used
		uint8_t attributeTile = (((y % 32) / 16) << 1) + ((x % 32) / 16);
		uint8_t attribute = nametables[attributeIndex];

		// now get the palette offset from attribute
		uint8_t paletteOffset = attribute & 3;
		if (attributeTile == 1)
			paletteOffset = (attribute >> 2) & 3;
		else if (attributeTile == 2)
			paletteOffset = (attribute >> 4) & 3;
		else if (attributeTile == 3)
			paletteOffset = (attribute >> 6) & 3;

		// since background palettes start at 0x3f00 and are 4 bytes each, 1 for each color
		int paletteIndex = 0x3f00 + 4*paletteOffset;

		// decode the colors
		// return their rgba values
		uint8_t** rgbaVals = 0;
		return rgbaVals;
	}

	// draws a 8x8 pixel region, pattern0 takes the 8byte pattern segment that affects bit 0 of the color
	void drawTile(int x, int y, uint64_t pattern0, uint64_t pattern1, uint8_t** colors) {
		SDL_RenderClear(renderer);
		// draw pixels
		for (int r = 0; r < 8; r++)
			for (int c = 0; c < 8; c++) {
				uint8_t index = r * 8 + c;
				uint8_t color = (((pattern1 >> index) & 1) << 1) + (pattern0 >> index);
				// TODO uncomment these lines when colors is properly created
				//SDL_SetRenderDrawColor(renderer, colors[color][0], colors[color][1], colors[color][2], colors[color][3]);
				//SDL_RenderDrawPoint(renderer, x + c, y + r);
			}
	}

	// dummy implementation for rendering only the background
	void drawBackground() {
		for (int r = 0; r < 30; r++)
			for (int c = 0; c < 32; c++) {
				uint16_t background = (nametables[r*30 + c] + (((uint16_t) PPUCTRL) << 8) * 2);
				int x = c * 8;
				int y = r * 8;
				uint8_t** colors = readBackgroundPalette(x, y);
				uint64_t pattern0 = chrom[background];
				uint64_t pattern1 = chrom[background + 1];
				drawTile(x, y, pattern0, pattern1, colors);
			}
	}

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
