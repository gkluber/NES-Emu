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
};

namespace Core
{		
	extern uint16_t pc; // Program counter
	extern uint8_t sp; // Stack pointer
	
	extern int8_t a; // Accumulator
	extern int8_t x; // Index register X
	extern int8_t y; // Index register Y
	
	extern int8_t* data;
	
	// Mapper number that we use to configure memory
	// TODO abstract memory so that arbitrary mappers work
	extern uint8_t mapper;
	extern uint8_t prg_rom_size;
	
	// Memory
	extern uint8_t ram[2048];
	extern uint8_t wram[8196];
	extern uint8_t rom[32784];

	// Interrupt vectors	
	extern uint16_t NMI;
	extern uint16_t RESET;
	extern uint16_t IRQ;
	
	extern uint64_t cyc;
	
	extern Flags p;
		
	void execute();
	void dumpcore();	
	
	void power();
	void reset();
};
