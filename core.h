#pragma once

#include <cstdint>
#include "util.h"
#include "flags.h"


struct Flags
{
	bool n; // Negative
	bool v; // Overflow
	bool d; // Decimal
	bool i; // Interrupt Disable
	bool z; // Zero
	bool c; // Carry
};

namespace Core
{		
	uint16_t pc; // Program counter
	uint8_t sp; // Stack pointer
	
	uint8_t a; // Accumulator
	uint8_t x; // Index register X
	uint8_t y; // Index register Y
	
	uint8_t mem[65536];
	
	// Initialize flags to 0
	Flags p = {};
		
	void execute();
	void dumpcore();	
};
