#pragma once

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <SDL2/SDL.h>

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
		
	void power();
	void reset();
	void drawBackground();
}
