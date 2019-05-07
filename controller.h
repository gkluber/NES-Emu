#pragma once
#include <stdint.h>
#include <SDL2/SDL.h>

namespace CONTROLLER {

	uint8_t pollController(SDL_GameController* control);
	void set_controller_strobe(bool s);
	void power();
	void reset();
}
