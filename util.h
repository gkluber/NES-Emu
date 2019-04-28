#pragma once

#include <cstdint>
#include <iostream>
#include "flags.h"

// Range of bits between start and end, exclusive of end and inclusive of start
inline uint32_t bitrange(const uint32_t *num, const uint8_t start, const uint8_t end)
{
	if(DEBUG)
	{
		if(end > 32 || start < 0 || start >= end)
		{
			std::cout << "FATAL: WARNING BITRANGE RECEIVED BAD ARGUMENTS" << std::endl;
			exit(1);
			return 0;
		}
	}
	uint8_t endshift = 32 - end;
	return (*num << endshift) >> (endshift + start);
}

// Needed to make signextend32, signextend64 compatible with other template functions
template <class T>
T signextend(T num, const uint8_t msb) {} 

static const uint32_t extension32 = 0x7fffffff;
template<> inline uint32_t signextend(uint32_t num, const uint8_t msb)
{
	if(DEBUG && msb >= 32)
	{
		std::cout << "FATAL: WARNING SIGNEXTEND32 RECEIVED BAD ARGUMENTS" << std::endl;
		exit(1);
	}
	
	if(num >> msb)
		num |= extension32 << (msb + 1);	
	//const uint32_t extension = 0x7FFFFFFF << msb;
	//if(num & extension)
	//	num |= (extension << 1);
	return num;
}

static const uint64_t extension64 = 0x7fffffffffffffff; 
template<> inline uint64_t signextend(uint64_t num, const uint8_t msb)
{
	if(DEBUG && msb >= 64)
	{
		std::cout << "FATAL: WARNING SIGNEXTEND64 RECEIVED BAD ARGUMENTS" << std::endl;
		exit(1);
	}

	if(num >> msb)
		num |= extension64 << (msb + 1);
	return num;
}

template<class T>
inline T ROR(const T num, const uint8_t amt, const uint8_t size)
{
	//if(amt == 0)
	//	return num;
	return num >> amt | num << (size - amt);
}

template <class T>
T apply_shift(T num, const uint8_t shift, const uint8_t amt) {} 

template<> inline uint64_t apply_shift(uint64_t num, const uint8_t shift, const uint8_t amt)
{
	switch(shift)
	{
		case 0b00:
		return num << amt;
		case 0b01:
		return num >> amt;
		case 0b10:
		return signextend<uint64_t>(num >> amt, 63 - amt);	
		case 0b11:
		return ROR<uint64_t>(num, amt, 64);
	}
	return 0;
}

template<> inline uint32_t apply_shift(uint32_t num, const uint8_t shift, const uint8_t amt)
{
	switch(shift)
	{
		case 0b00:
		return num << amt;
		case 0b01:
		return num >> amt;
		case 0b10:
		return signextend<uint32_t>(num >> amt, 31 - amt);	
		case 0b11:
		return ROR<uint32_t>(num, amt, 32);
	}
	return 0;
}

template <class T>
inline T max(T t1, T t2)
{
	return t1 > t2 ? t1 : t2;
}
