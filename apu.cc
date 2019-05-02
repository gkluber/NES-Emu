#include "apu.h"

#include <cstdio>
#include <iostream>

using namespace APU;

namespace APU
{	
	// Pulse channel 1
	uint8_t SQ1_VOL;
	uint8_t SQ1_SWEEP;
	uint8_t SQ1_LO;
	uint8_t SQ1_HI;
		
	// Pulse channel 2
	uint8_t SQ2_VOL;
	uint8_t SQ2_SWEEP;
	uint8_t SQ2_LO;
	uint8_t SQ2_HI;
	
	// Triangle channel 
	uint8_t TRI_LINEAR;
	uint8_t TRI_LO; 
	uint8_t TRI_HI;
	
	// Noise channel
	uint8_t NOISE_VOL; 
	uint8_t NOISE_LO;
	uint8_t NOISE_HI; 
	
	// DMC channel
	uint8_t DMC_FREQ;
	uint8_t DMC_RAW;
	uint8_t DMC_START;
	uint8_t DMC_LEN;
	uint8_t DMC_CTRL;
	uint8_t DMC_STATUS; 
	uint8_t DMC_FCOUNTER; 
	
		
	void power()
	{
		//TODO
	}
	
	void reset()
	{
		//TODO
	}
}
