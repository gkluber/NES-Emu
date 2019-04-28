#include <iostream>
#include <cstdio>
#include <cstring>
#include <stdint.h>

#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char *argv[])
{
	if(argc == 1 || strcmp(argv[1], "gui") == 0)
	{
		SDL_Window *window = nullptr;
		
		SDL_Surface *screen = nullptr;
		
		int result = SDL_Init(SDL_INIT_VIDEO);	
		if(result < 0)
		{
			printf("SDL failed to initialize: %s\n", SDL_GetError());
			return 0;
		}
		
		window = SDL_CreateWindow("NES Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if(window == nullptr)
		{
			printf("Could not create window: %s\n", SDL_GetError());
			return 0;
		}
		
		screen = SDL_GetWindowSurface(window);
		
		SDL_FillRect(screen, nullptr, SDL_MapRGB(screen->format, 0xff, 0xff, 0xff));
		
		SDL_UpdateWindowSurface(window);
		
		SDL_Delay(2000);
		
		SDL_DestroyWindow(window);
		
		SDL_Quit();	
	}
	else if(strcmp(argv[1], "mapper") == 0)
	{
		// Test mapper
	}
	else if(strcmp(argv[1], "cpu") == 0)
	{
		// Test CPU
	}
}


