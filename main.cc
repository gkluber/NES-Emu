#include "mapper.h"
#include "core.h"
#include "ppu.h"
#include "controller.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <stdint.h>

#include <SDL2/SDL.h>

const unsigned int IMG_WIDTH = 256;
const unsigned int IMG_HEIGHT = 240;
const unsigned int SCREEN_WIDTH = 640;
const unsigned int SCREEN_HEIGHT = 480;

int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		std::cout << "Not enough arguments!" << std::endl;
	}
	else if(strcmp(argv[1], "gui") == 0)
	{
		int result = SDL_Init(SDL_INIT_VIDEO);	
		if(result < 0)
		{
			printf("SDL failed to initialize: %s\n", SDL_GetError());
			return 0;
		}
		
		SDL_Init(SDL_INIT_VIDEO);
		SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN, &window, &renderer);
		if(window == nullptr)
		{
			printf("Could not create window: %s\n", SDL_GetError());
			return 0;
		}
		
		screen = SDL_GetWindowSurface(window);
		
		PPU::drawBackground();
		SDL_RenderPresent(renderer);
		
		//SDL_UpdateWindowSurface(window);
		
		// Read file
		if(argc != 3)
		{
			std::cout << "Format: ./nes gui <filename>" << std::endl;
			return 0;
		}
		read_ines(argv[2]);
		
		// Power NES
		Core::power();
		PPU::power();
		
		SDL_Event e;
		uint8_t kbState = 0; //address $4016 w/ input state
		//GAME LOOP ---------------
		while (true) {
			//handle events and update the state of the keyboard for when the cpu requests it
			while (SDL_PollEvent(&e)) {
				if (e.type == SDL_KEYDOWN) {
					printf("KEY PRESSED\n");
					switch(e.key.keysym.sym) {
						case SDLK_k: {
							kbState |= 1;
							break;
						}
						case SDLK_j: {
							kbState |= (1<<1);
							break;
						}
						case SDLK_l: {
							kbState |= (1<<2);
							break;
						}
						case SDLK_RETURN: {
							kbState |= (1<<3);
							break;
						}
						case SDLK_w: {
							kbState |=(1<<4);
							break;
						}
						case SDLK_s: {
							kbState |= (1<<5);
							break;
						}
						case SDLK_a: {
							kbState |= (1<<6);
							break;
						}
						case SDLK_d: {
							kbState |= (1<<7);
							break;
						}
						case SDLK_ESCAPE: {
								return 0; //quit program
								break;
						}
					}
				}
				if (e.type == SDL_KEYUP) {
					printf("KEY RELEASED\n");
					switch(e.key.keysym.sym) {
						case SDLK_k: {
							kbState &= ~1;
							break;
						}
						case SDLK_j: {
							kbState &= ~(1<<1);
							break;
						}
						case SDLK_l: {
							kbState &= ~(1<<2);
							break;
						}
						case SDLK_RETURN: {
							kbState &= ~(1<<3);
							break;
						}
						case SDLK_w: {
							kbState &= ~(1<<4);
							break;
						}
						case SDLK_s: {
							kbState &= ~(1<<5);
							break;
						}
						case SDLK_a: {
							kbState &= ~(1<<6);
							break;
						}
						case SDLK_d: {
							kbState &= ~(1<<7);
							break;
						}
					}
				}
				if (e.type == SDL_QUIT) {
					printf("QUIT event\n");
					return 0; //QUIT PROGRAM
				}
			}
			if (kbState !=0) {
				//printf("Keyboard state is %d\n", kbState);
			}
		}	
	}
	else if(strcmp(argv[1], "cpu") == 0)
	{
		if(argc != 3)
		{
			std::cout << "Format: ./nes cpu <filename>" << std::endl;
			return 0;
		}
		read_ines(argv[2]);
		Core::power();
		Core::execute();
	} else if (strcmp(argv[1], "controller") == 0) {
		printf("---Welcome to Controller Testing Mode---\n");
		SDL_Init(SDL_INIT_EVERYTHING);
		CONTROLLER::power();
		SDL_Event e;
		while (true) {
			//handler events
			int32_t ctrlID = -1;
			SDL_GameController* control = 0;
			while (SDL_PollEvent(&e)) {
				if (e.type == SDL_CONTROLLERDEVICEADDED) {
					printf("DEVICE ADDED EVENT\n");
					ctrlID = e.cdevice.which;
					control = SDL_GameControllerOpen(ctrlID);

				}
				if (e.type == SDL_QUIT) {
					printf("QUIT event\n");
					return 0; //QUIT PROGRAM
				}
			}
			
			uint8_t ctrlIn = CONTROLLER::pollController(control);
			if (ctrlIn != 0) {
				printf("Controller state = %d", ctrlIn);
			}
		}
	}
	return 0;
}


