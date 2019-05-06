#include <stdint.h>
#include <SDL2/SDL.h>


namespace CONTROLLER {
		
	uint8_t pollController (SDL_GameController* control) {
	    if (control == NULL) {
				return 0;
		}
		printf("not null controller\n");
		SDL_GameControllerUpdate();
		uint8_t state = 0;
		state += (SDL_GameControllerGetButton(control, SDL_CONTROLLER_BUTTON_A)) << 0;
		state += (SDL_GameControllerGetButton(control, SDL_CONTROLLER_BUTTON_B)) << 1;
		state += (SDL_GameControllerGetButton(control, SDL_CONTROLLER_BUTTON_GUIDE)) << 2;
		state += (SDL_GameControllerGetButton(control, SDL_CONTROLLER_BUTTON_START)) << 3;
		state += (SDL_GameControllerGetButton(control, SDL_CONTROLLER_BUTTON_DPAD_UP)) << 4;
		state += (SDL_GameControllerGetButton(control, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) << 5;
		state += (SDL_GameControllerGetButton(control, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) << 6;
		state += (SDL_GameControllerGetButton(control, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) << 7;
		return state;
	}



	void power() {
    	SDL_Init(SDL_INIT_GAMECONTROLLER);
		int32_t ctrlNum = SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
		if (ctrlNum == -1) {
			printf("BAD, SDL error\n");
		}
		printf("Here's the ctrl num: %d\n", ctrlNum);
		SDL_GameControllerEventState(SDL_IGNORE);
	}
	
	void reset() {
	}
}
