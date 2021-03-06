#pragma once

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <SDL2/SDL.h>
#include "core.h"

extern SDL_Window *window;		
extern SDL_Surface *screen;
extern SDL_Renderer *renderer;		

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
	
	extern uint64_t patterns[2][512];
	extern uint8_t nametables[4][1024];
	extern uint8_t palettes[8][4];
	extern uint8_t OAM[64][4];
	extern uint8_t secOAM[8][4];
		
	void writePPUCTRL();
	void writePPUMASK();
	void writeOAMADDR();
	void writeOAMDATA();
	void writePPUSCROLL();
	void writePPUADDR();
	void writePPUDATA();
	uint8_t* readPPUDATA();
	void writeOAMDMA();
		
	uint8_t** readBackgroundPalette(int x, int y);
		// find attribute in memory for the segment of the window at coordinates x,y
	void power();
	void reset();
	void drawBackground();
	void randomizeGUI();
}
