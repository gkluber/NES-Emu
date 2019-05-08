#include "ppu.h"

#define NAMETABLE_SIZE 0x03c0

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

	uint8_t colors[256][4];

	uint64_t patterns[2][512];
	uint8_t nametables[4][1024];
	uint8_t palettes[8][4];
	uint8_t OAM[64][4];
	uint8_t secOAM[8][4];

	void initColors() {
		for (int i = 0; i < 256; i++) {
			for (int j = 0; j < 3; j++) {
				colors[i][j] = rand() % 256;
			}
			colors[i][3] = 255;
		}
	}

	void writePPUCTRL() {

	}
	void writePPUMASK() {

	}
	void writeOAMADDR() {

	}
	void writeOAMDATA() {

	}
	void writePPUSCROLL() {

	}
	void writePPUADDR() {

	}
	void writePPUDATA() {

	}
	void writeOAMDMA() {

	}
		
	uint8_t** readBackgroundPalette(int x, int y) {
		// find attribute in memory for the segment of the window at coordinates x,y
		// this will change, 960 refers to last address of the nametable it follows, x and y offset correct
		int attributeIndex = x / 32 + (y / 32 * 8);
		// find which of the 4 4x4pixel tiles is used
		uint8_t attributeTile = (((y % 32) / 16) << 1) + ((x % 32) / 16);
		uint8_t attribute = nametables[PPUCTRL & 3][NAMETABLE_SIZE + attributeIndex];

		// now get the palette offset from attribute
		uint8_t paletteOffset = attribute & 3;
		if (attributeTile == 1)
			paletteOffset = (attribute >> 2) & 3;
		else if (attributeTile == 2)
			paletteOffset = (attribute >> 4) & 3;
		else if (attributeTile == 3)
			paletteOffset = (attribute >> 6) & 3;

		// since background palettes are first, before sprite palettes no offset needed, 1 byte for each color

		// decode the colors
		// return their rgba values
		uint8_t** rgbaVals = new uint8_t*[4];
		for (int i = 0; i < 4; i++) {
			rgbaVals[i] = new uint8_t[4];
			for (int j = 0; j < 4; j++)
				rgbaVals[i][j] = colors[palettes[paletteOffset][i]][j];
		}
		return rgbaVals;
	}

	// draws a 8x8 pixel region, pattern0 takes the 8byte pattern segment that affects bit 0 of the color
	void drawTile(int x, int y, uint64_t pattern0, uint64_t pattern1, uint8_t** colors) {
		//printf("%x\n", 
		//SDL_RenderClear(renderer);
		// draw pixels
		for (int r = 0; r < 8; r++)
			for (int c = 0; c < 8; c++) {
				uint8_t index = r * 8 + c;
				uint8_t color = ((((pattern1 >> index) & 1) << 1) + (pattern0 >> index)) & 3;
				// TODO uncomment these lines when colors is properly created
				SDL_RenderClear(renderer);
				SDL_SetRenderDrawColor(renderer, colors[color][0], colors[color][1], colors[color][2], colors[color][3]);
				//SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
				SDL_RenderDrawPoint(renderer, x + c, y + r);
			}
	}

	// dummy implementation for rendering only the background
	void drawBackground() {
		for (int r = 0; r < 30; r++) {
			for (int c = 0; c < 32; c++) {
				uint16_t background = (nametables[PPUCTRL & 3][r*32 + c] * 2);
				int x = c * 8;
				int y = r * 8;
				uint8_t** colors = readBackgroundPalette(x, y);
				uint64_t pattern0 = patterns[(PPUCTRL >> 3) & 1][background];
				uint64_t pattern1 = patterns[(PPUCTRL >> 3) & 1][background + 1];
				drawTile(x, y, pattern0, pattern1, colors);
			}
		}
	}

	inline void resetSecondaryOAM()
	{
		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 4; j++)
				secOAM[i][j] = 0xff;
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
		initColors();
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
