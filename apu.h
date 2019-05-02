#pragma once

#include <cstdint>

namespace APU
{	
	// All of the following registers are write-only except for dmc_status ($4015)
	// Pulse channel 1
	// DDLC NNNN, D = duty, L = loop (length counter), C = constant volume, N = envelope period/volume
	extern uint8_t SQ1_VOL;
	// Pulse channel sweep unit:
	// EPPP NSSS, E = enabled, P = period, N = negative, S = shift count
	extern uint8_t SQ1_SWEEP;
	// LLLL LLLL, L = timer low
	extern uint8_t SQ1_LO;
	// LLLL LHHH, L = length counter load, H = timer high
	extern uint8_t SQ1_HI;
		
	// Pulse channel 2
	// DDLC NNNN, D = duty, L = loop (length counter), C = constant volume, N = envelope period/volume
	extern uint8_t SQ2_VOL;
	// Pulse channel sweep unit:
	// EPPP NSSS, E = enabled, P = period, N = negative, S = shift count
	extern uint8_t SQ2_SWEEP;
	// LLLL LLLL, L = timer low
	extern uint8_t SQ2_LO;
	// LLLL LHHH, L = length counter load, H = timer high
	extern uint8_t SQ2_HI;
	
	// Triangle channel 
	// CRRR RRRR, C = length/linear counter enable/disable, R = linear counter reload value
	extern uint8_t TRI_LINEAR;
	// LLLL LLLL, L = timer low
	extern uint8_t TRI_LO; // Mapped to $400A
	// LLLL LHHH, L = length counter load, H = timer high
	extern uint8_t TRI_HI;
	
	// Noise channel
	// --LC NNNN, - = unused, L = loop, C = constant volume, N = envelope period/volume
	extern uint8_t NOISE_VOL; 
	// L--- PPPP, L = loop noise, P = period
	extern uint8_t NOISE_LO;
	// LLLL L---, L = length counter load
	extern uint8_t NOISE_HI; // $400F
	
	// DMC channel
	// IL-- FFFF, I = IRQ enable, L = loop sample, F = frequency index
	extern uint8_t DMC_FREQ;
	// -DDD DDDD, D = direct load
	extern uint8_t DMC_RAW;
	// AAAA AAAA, A = sample address
	extern uint8_t DMC_START;
	// LLLL LLLL, L = sample length
	extern uint8_t DMC_LEN; // $4013
	
	// SND_CHN registers
	// ---D NT21, D = DMC enable, N, T, 2, 1 = len counter enables
	extern uint8_t DMC_CTRL; // mapped to CPU $4015 writes
	// IF-D NT21, I = DMC interrupt, F = frame interrupt, D = DMC enable?, N, T, 2, 1 len counter status (enabled bits?)
	extern uint8_t DMC_STATUS; // $4015 reads
	
	// JOY2 writes ($4017)
	// SD-- ----, S = 5-frame sequence, D = disable frame interrupt
	extern uint8_t DMC_FCOUNTER; // Frame counter
	
		
	void power();
	void reset();
}
