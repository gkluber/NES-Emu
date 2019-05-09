#include "ppu.h"

#define PATTERN_SIZE 0x1000
#define NAMETABLE_START 0x2000
#define NAMETABLE_SIZE 0x03c0
#define NAMETABLE_BLOCK_SIZE 0x0400
#define PALETTE_START 0x3f00
#define PALETTE_SIZE 0x0020

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
	bool firstPPUADDRwrite = true;
	uint8_t PPUDATAbuffer;
	uint8_t PPUDATAbufferbuffer;
	uint8_t PPUDATAmutable;

	uint8_t colors[256][4];

	uint64_t patterns[2][512];
	uint8_t nametables[4][1024];
	uint8_t palettes[8][4];
	uint8_t OAM[64][4];
	uint8_t secOAM[8][4];

	void initColors() {
		srand(time(0));
		for (int i = 0; i < 256; i++) {
			for (int j = 0; j < 3; j++) {
				colors[i][j] = rand() % 256;
			}
			colors[i][3] = 255;
		}
	}

	// writes data to current vram address
	void writeVRAM(uint8_t data) {
		v %= 0x4000;
		// if writing to patterns table
		if (v < NAMETABLE_START) {
			uint8_t* patts = (uint8_t*) patterns[v / PATTERN_SIZE];
			patts[v % PATTERN_SIZE] = data;
		} else if (v < PALETTE_START) {
			printf("nametable write %d at %x\n", data, v);
			// if writing to nametables
			// % 4 since the 5th nametable is a mirror of the first
			uint8_t* names = nametables[((v - NAMETABLE_START) / NAMETABLE_BLOCK_SIZE) % 4];
			names[v % NAMETABLE_BLOCK_SIZE] = data;
		} else {
			printf("palette write %d at %x\n", data, v);
			// else writing to palettes
			// % 8 since 0x3f20-0x3fff are mirrors of 0x3f00-0x3f1f
			uint8_t* pals = palettes[((v - PALETTE_START) / 4) % 8];
			pals[v % 4] = data;
		}
	}

	// reads data from current vram address
	uint8_t readVRAM() {
		v %= 0x4000;
		// if reading from patterns table
		if (v < NAMETABLE_START) {
			uint8_t* patts = (uint8_t*) patterns[v / PATTERN_SIZE];
			return patts[v % PATTERN_SIZE];
		} else if (v < PALETTE_START) {
			printf("nametable read at %x\n", v);
			// if reading from nametables
			// % 4 since the 5th nametable is a mirror of the first
			uint8_t* names = nametables[((v - NAMETABLE_START) / NAMETABLE_BLOCK_SIZE) % 4];
			return names[v % NAMETABLE_BLOCK_SIZE];
		} else {
			printf("palette read at %x\n", v);
			// else reading from palettes
			// % 8 since 0x3f20-0x3fff are mirrors of 0x3f00-0x3f1f
			uint8_t* pals = palettes[((v - PALETTE_START) / 4) % 8];
			return pals[v % 4];
		}
	}

	void writePPUCTRL() {
		printf("PPUCTRL altered: %x\n", PPUCTRL);
	}
	void writePPUMASK() {

	}
	void writeOAMADDR() {

	}
	void writeOAMDATA() {

	}
	void writePPUSCROLL() {

	}
	// TODO verify this directly alters vram
	void writePPUADDR() {
		if (firstPPUADDRwrite) {
			v = ((uint16_t) PPUADDR) << 8;
			firstPPUADDRwrite = false;
		} else {
			v += PPUADDR;
			firstPPUADDRwrite = true;
			printf("vaddr: %x\n", v);
		}
	}
	void writePPUDATA() {
		if ((PPUCTRL >> 2) & 1 == 0)
			v -= 1;
		else
			v -= 32;
		PPUDATAbuffer = PPUDATAbufferbuffer;
		writeVRAM(PPUDATA);
		// increment based on PPUCTRL
		if ((PPUCTRL >> 2) & 1 == 0)
			v += 1;
		else
			v += 32;
	}
	uint8_t* readPPUDATA() {
		PPUDATAbufferbuffer = PPUDATAbuffer;
		PPUDATAmutable = PPUDATAbuffer;
		PPUDATAbuffer = readVRAM();
		if (v >= PALETTE_START)
			PPUDATAmutable = PPUDATAbuffer;
		if ((PPUCTRL >> 2) & 1 == 0)
			v += 1;
		else
			v += 32;
		return &PPUDATAmutable;
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

	void printPals() {
		for (int i = 0; i < 8; i++) {
			printf("%d, %d, %d, %d\n", palettes[i][0], palettes[i][1], palettes[i][2], palettes[i][3]);
		}
	}

	// dummy implementation for rendering only the background
	void drawBackground() {
		printPals();
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
