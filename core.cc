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
		
	
	///////////////////////////
	// Register instructions //
	///////////////////////////
	
	// all take exactly 2 cycles
	// Affect the flags N and Z
	
	inline void TAX()
	{
		x = a;
		p.z = x == 0;
		p.n = x < 0;
	}
	
	inline void TXA()
	{
		a = x;
		p.z = a == 0;
		p.n = a < 0;
	}
	
	inline void DEX()
	{ 
		x--;
		p.z = x == 0;
		p.n = x < 0;
	}
	
	inline void INX()
	{	
		x++;
		p.z = x == 0;
		p.n = x < 0;
	}
	
	inline void TAY()
	{
		y = a;
		p.z = y == 0;
		p.n = y < 0;
	}
	
	inline void TYA()
	{
		a = y;
		p.z = a == 0;
		p.n = a < 0;
	}
	
	inline void DEY()
	{
		y--;
		p.z = y == 0;
		p.n = y < 0;
	}

	inline void INY()
	{
		y++;
		p.z = y == 0;
		p.n = y < 0;
	}
	
	/////////////////////////
	/// Flag Instructions ///
	/////////////////////////

	inline void CLC()
	{
		p.c = 0;
	}
	inline void CLD()
	{
		p.d = 0;
	}

	inline void CLI()
	{
		p.i = 0;
	}

	inline void CLV()
	{
		p.v = 0;	
	}

	inline void SEC()
	{
		p.c = 1;
	}

	inline void SED()
	{
		p.d = 1;
	}

	inline void SEI()
	{
		p.i = 1;
	}
	
	//////////////////////////
	/// Stack Instructions ///
	//////////////////////////

	inline void TXS()
	{
		//
	}
	
	inline void TSX()
	{
		//
	}

	inline void PHA()
	{
		//
	}
	
	inline void PLA()
	{
		//
	}

	inline void PHP()
	{
		//
	}

	inline void PLP()
	{
		//
	}

	int8_t bob;

	// addressing mode functions, to be called before instructions, update the global pointer data
	void accMode() {
		data = *a;
	}
	void immMode() {
		bob = mem[pc+1];
		data = *bob;
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

				// register instructions
				case 0xaa:
				{
					pc++;
					TAX();
					break;
				}
                case 0x8a:
                {
                    pc++;
                    TXA();
                    break;
                }
                case 0xca:
                {
                    pc++;
                    DEX();
                    break;
                }
                case 0xe8:
                {
                    pc++;
                    INX();
                    break;
                }
				case 0xa8:
				{
					pc++;
					TAY();
					break;
				}
                case 0x98:
                {
                    pc++;
                    TYA();
                    break;
                }
                case 0x88:
                {
                    pc++;
                    DEY();
                    break;
                }

                case 0xc8:
                {
                    pc++;
                    INY();
                    break;
                }

				// bitwise OR with Accumulator
				case 0x09:
				{
					pc+=2;
					ORA();
					break;
				}
				case 0x0d:
				{
					pc += 3;
					ORA();
					break;
				}

				//NOP
				case 0xea:
				{
					pc++;	
					break;
				}
				
				//Flag (Processor Status) Instructions
                case 0x18:
				{
					pc++;
					CLC();
					break;
				}	
                case 0x38:
				{
					pc++;
					SEC();
					break;
				}
                case 0x58:
				{
					pc++;
					CLI();
					break;
				}
                case 0x78:
				{
					pc++;
					SEI();
					break;
				}
                case 0xb8:
				{
					pc++;
					CLV();
					break;
				}
                case 0xd8:
				{
					pc++;
					CLD();
					break;
				}
                case 0xf8:
				{
					pc++;
					SED();
					break;
				}

				//Stack Instructions
				case 0x9a:
				{
					pc++;
					TXS();
					break;
				}
				case 0xba:
				{
					pc++;
					TSX();
					break;
				}
				case 0x48:
				{
					pc++;
					PHA();
					break;
				}
				case 0x68:
				{
					pc++;
					PLA();
					break;
				}
				case 0x08:
				{
					pc++;
					PHP();
					break;
				}
				case 0x28:
				{
					pc++;
					PLP();
					break;
				}

				//Branch Instructions
				



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
