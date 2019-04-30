#include "core.h"
#include "flags.h"

#include <cstdio>
#include <iostream>



using namespace Core;

namespace Core
{
	// Instantiate global variables from core.h
	uint16_t pc;
	uint8_t sp;
	
	int8_t a;
	int8_t x;
	int8_t y;
	
	uint8_t mem[65536];
	int8_t *data;
	
	Flags p = {};
	
	
	/////////////////////////
	// Branch instructions //
	/////////////////////////
		
	inline void BPL(int8_t d)
	{
		if(!p.n)
			pc += d;	
	}
	
	inline void BMI(int8_t d)
	{
		if(p.n)
			pc += d;	
	}

	inline void BVC(int8_t d)
	{
		if(!p.v)
			pc += d;	
	}
	
	inline void BVS(int8_t d)
	{
		if(p.v)
			pc += d;	
	}
	
	inline void BCC(int8_t d)
	{
		if(!p.c)
			pc += d;	
	}
	
	inline void BCS(int8_t d)
	{
		if (p.c)
			pc += d;	
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
		sp = x;
	}
	
	inline void TSX()
	{
		x = sp;
		p.z = sp == 0;
		p.n = ((int8_t)sp) < 0;
	}

	inline void PHA()
	{
		mem[sp] = a;
		sp--;
	}
	
	inline void PLA()
	{
		sp++;
		a = mem[sp];
		p.z = sp == 0;
		p.n = ((int8_t)sp) <0;
	}

	inline void PHP()
	{
		uint8_t statusReg = (p.n<<7) + (p.v<<6) + (p.b<<4) + (p.d<<3) + (p.i<<2) + (p.z<<1) + p.c;
		mem[sp] = statusReg;
		sp--; 
	}

	inline void PLP()
	{
		sp++;
		uint8_t statusReg = mem[sp];
		p.n = 0 != ((((uint8_t)1)<<7) & statusReg);
		p.v = 0 != ((((uint8_t)1)<<6) & statusReg);
		p.b = 0 != ((((uint8_t)1)<<4) & statusReg);
		p.d = 0 != ((((uint8_t)1)<<3) & statusReg);
		p.i = 0 != ((((uint8_t)1)<<2) & statusReg);
		p.z = 0 != ((((uint8_t)1)<<1) & statusReg);
		p.c = 0 != ((((uint8_t)1)) & statusReg);
	}

	int8_t bob;

	// addressing mode functions, to be called before instructions, update the global pointer data
	// accumulator mode
	void accMode() {
		data = &a;
	}
	// immediate mode
	void immMode() {
		bob = mem[pc+1];
		data = &bob;
	}
	// absolute mode
	void absMode() {
		uint16_t addr = (mem[pc+2] << 8) + mem[pc+1];
		data = (int8_t *) &mem[addr];
	}
	// zero-page mode
	void zrpMode() {
		uint8_t addr = mem[pc+1];
		data = (int8_t *) &mem[addr];
	}
	// absolute indexed mode using x
	void absix() {
		// TODO verify that x remains unchanged
		uint16_t addr = ((uint8_t) x) + (mem[pc+2] << 8) + mem[pc+1];
		data = (int8_t *) &mem[addr];
	}
	// absolute indexed mode using y
	void absiy() {
		// TODO verify that y remains unchanged
		uint16_t addr = ((uint8_t) y) + (mem[pc+2] << 8) + mem[pc+1];
		data = (int8_t *) &mem[addr];
	}
	// zero-paged indexed mode using x
	void zrpix() {
		// TODO verify that x remains unchanged
		uint16_t addr = ((uint8_t) x) + mem[pc+1];
		data = (int8_t *) &mem[addr];
	}
	// zero-paged indexed mode using y
	void zrpiy() {
		// TODO verify that y remains unchanged
		uint16_t addr = ((uint8_t) y) + mem[pc+1];
		data = (int8_t *) &mem[addr];
	}
	// pre-indexed indirect mode (uses x)
	void iix() {
		//x += mem[pc+1]; TODO verify x unchanged
		uint16_t addr = (mem[((uint8_t) x) + mem[pc+1] + 1] << 8) + mem[((uint8_t) x) + mem[pc+1]];
		data = (int8_t *) &mem[addr];
	}
	// post-indexed indirect mode (uses y)
	void iiy() {
		uint16_t imm = mem[pc+1];
		uint16_t addr = (mem[imm+1] << 8) + mem[imm] + ((uint8_t) y);
		data = (int8_t *) &mem[addr];
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
				//branch instructions
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
				case 0x10:
				{
					pc += 2;
					BPL(mem[pc-1]);
					break;
				}
				case 0x30:
				{
					pc += 2;
					BMI(mem[pc-1]);
					break;
				}
				case 0x50:
				{
					pc += 2;
					BVC(mem[pc-1]);
					break;
				}
				case 0x70:
				{
					pc += 2;
					BVS(mem[pc-1]);
					break;
				}
				case 0x90:
				{
					pc += 2;
					BCC(mem[pc-1]);
					break;
				}
				case 0xb0:
				{
					pc += 2;
					BCS(mem[pc-1]);
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
