#pragma once

#include <cstdint>
#include "flags.h"


struct Flags
{
	bool n; // Negative
	bool v; // Overflow
	bool d; // Decimal
	bool i; // Interrupt Disable
	bool z; // Zero
	bool c; // Carry
	bool b; // Interrupt Control Bit
};

namespace Core
{		
	extern uint16_t pc; // Program counter
	extern uint8_t sp; // Stack pointer
	
	extern int8_t a; // Accumulator
	extern int8_t x; // Index register X
	extern int8_t y; // Index register Y
	
	extern uint8_t mem[65536];
	extern int8_t* data;
	
	// Initialize flags to 0
	extern Flags p;
		
	void execute();
	void dumpcore();	
};
