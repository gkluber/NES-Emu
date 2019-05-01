#pragma once

#include <cstdint>

namespace APU
{	
	// All of the following registers are write-only except for dmc_status ($4015)
	// Pulse channel 1
	// DDLC NNNN, D = duty, L = loop (length counter), C = constant volume, N = envelope period/volume
	extern uint8_t pulse1_env;
	// Pulse channel sweep unit:
	// EPPP NSSS, E = enabled, P = period, N = negative, S = shift count
	extern uint8_t pulse1_sweep;
	// LLLL LLLL, L = timer low
	extern uint8_t pulse1_timerlo;
	// LLLL LHHH, L = length counter load, H = timer high
	extern uint8_t addr4003;
		
	// Pulse channel 2
	// DDLC NNNN, D = duty, L = loop (length counter), C = constant volume, N = envelope period/volume
	extern uint8_t pulse2_env;
	// Pulse channel sweep unit:
	// EPPP NSSS, E = enabled, P = period, N = negative, S = shift count
	extern uint8_t pulse2_sweep;
	// LLLL LLLL, L = timer low
	extern uint8_t pulse2_timerlo;
	// LLLL LHHH, L = length counter load, H = timer high
	extern uint8_t addr4007;
	
	// Triangle channel 
	// CRRR RRRR, C = length/linear counter enable/disable, R = linear counter reload value
	extern uint8_t tri_env;
	// LLLL LLLL, L = timer low
	extern uint8_t tri_timerlo; // Mapped to $400A
	// LLLL LHHH, L = length counter load, H = timer high
	extern uint8_t addr400B;
	
	// Noise channel
	// --LC NNNN, - = unused, L = loop, C = constant volume, N = envelope period/volume
	extern uint8_t noise_env; 
	// L--- PPPP, L = loop noise, P = period
	extern uint8_t noise_ctrl;
	// LLLL L---, L = length counter load
	extern uint8_t noise_counterl; // $400F
	
	// DMC channel
	// IL-- FFFF, I = IRQ enable, L = loop sample, F = frequency index
	extern uint8_t addr4010;
	// -DDD DDDD, D = direct load
	extern uint8_t dmc_directl;
	// AAAA AAAA, A = sample address
	extern uint8_t dmc_sample_addr;
	// LLLL LLLL, L = sample length
	extern uint8_t dmc_sample_len; // $4013
	// ---D NT21, D = DMC enable, N, T, 2, 1 = len counter enables
	extern uint8_t dmc_ctrl; // mapped to CPU $4015 writes
	// IF-D NT21, I = DMC interrupt, F = frame interrupt, D = DMC enable?, N, T, 2, 1 len counter status (enabled bits?)
	extern uint8_t dmc_status; // $4015 reads
	// SD-- ----, S = 5-frame sequence, D = disable frame interrupt
	extern uint8_t dmc_fcounter; // Frame counter ($4017)	
	
		
	void power();
	void reset();
}
