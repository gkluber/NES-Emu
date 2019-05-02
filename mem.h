#pragma once

#include <cstdint>
#include <cstdio>
#include <iostream>

class Memory
{
	public:
		Memory() {} // empty constructor
		virtual uint8_t read(uint16_t addr) = 0;
		virtual void write(uint16_t addr, uint8_t value) = 0;
		virtual ~Memory() {} // empty destructor
};
