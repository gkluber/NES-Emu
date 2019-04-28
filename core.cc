#include "core.h"
#include "flags.h"

#include <cstdio>
#include <iostream>



using namespace Core;

namespace Core
{
	/////////////////////////
	// Branch instructions //
	/////////////////////////
		
	inline void BPL(int8_t d)
	{
		
	}
	
	inline void BMI(int8_t d)
	{
		
	}

	inline void BVC(int8_t d)
	{
		
	}
	
	inline void BVS(int8_t d)
	{
		
	}
	
	inline void BCC(int8_t d)
	{
		
	}
	
	inline void BCS(int8_t d)
	{
		
	}
	
	inline void BNE(int8_t d)
	{
		if(!p.z)
			pc += d;
	}
	
	inline void BEQ(int8_t d)
	{
		if(p.z)
			pc += d;
	}
		
	///////////////////////
	// Flag instructions //
	///////////////////////
	
	inline void CLC()
	{
		p.c = false;
	}

	inline void SEC()
	{
		
	}

	inline void CLI()
	{

	}
	
	inline void SEI()
	{
		
	}

	inline void CLV()
	{
		
	}
	
	inline void CLD()
	{
		
	}

	inline void SED()
	{
		
	}
	
	///////////////////////////
	// Register instructions //
	///////////////////////////
	
	// Affect the flags N and Z
	
	inline void TAX()
	{
		a = x;
		if(a == 0)
			p.z = true;
		if(a < 0)
			p.n = true;
	}
	
	inline void TXA()
	{
		
	}
	
	inline void DEX()
	{
		
	}
	
	inline void INX()
	{
		
	}
	
	inline void TAY()
	{
		a = y;
		if(a == 0)
			p.z = true;
		if(a < 0)
			p.n = true;
	}
	
	inline void TYA()
	{
		
	}
	
	inline void DEY()
	{
		
	}

	inline void INY()
	{
		
	}
		
	void execute()
	{
		// TODO will need to accept interrupts from the system and from the PPU/APU
		// TODO will also need to work with the SDL2 GUI
		while(true)
		{
			uint8_t opcode = mem[pc]; 
			if(DEBUG)
				printf("Reading instruction %x on line %lx\n", opcode, pc);
				
			switch(opcode)
			{
				case 0xd0:
				{
					pc += 2;
					BNE(mem[pc-1]);
					break;
				}
				case 0xf0:
				{
					pc += 2;
					BEQ(mem[pc-1]);
					break;
				}
				case 0x18:
				{
					pc++;
					CLC();
					break;
				}
				case 0xaa:
				{
					pc++;
					TAX();
					break;
				}
				case 0xa8:
				{
					pc++;
					TAY();
					break;
				}
				default:
				{
					printf("Encountered invalid opcode %x on line %x!\n", opcode, pc);
					dumpcore();
				} 
			}	
		}	
	}

	void dumpcore()
	{
		printf("Core dump:\n");
		printf("PC: %x\n", pc);
		printf("SP: %x\n", sp);
		printf("A: %x\n", a);
		printf("X: %x\n", x);
		printf("Y: %x\n", y);
		printf("Flags: N=%d, V=%d, D=%d, I=%d, Z=%d, C=%d\n", p.n, p.v, p.d, p.i, p.z, p.c);
	}
}
