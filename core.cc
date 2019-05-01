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

	uint64_t cyc;
	
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

	//AND Bitwise AND with Accumulator
	inline void AND() {
		a = a & (*data);
		p.n = a<0;
		p.z = a == 0;
	}

	//ASL Arithmetic Shift Left
	inline void ASL() {
		p.c = (*data)<0;
		*data = (*data) << 1;
		p.n = (*data)<0;
		p.z = (*data)==0;
	}

	//STA Store Accumulator in Memory
	inline void STA () {
		mem[(*data)] = a;
	}

	//STX Store Index Register X in Memory
	inline void STX () {
		mem[(*data)] = x;
	}

	//STY Store Index Register Y in Memory
	inline void STY () {
		mem[(*data)] = y;
	}

	//CMP Compare Memory with Accumulator
	//TODO, check that I detected carry properly
	inline void CMP () {
		int8_t res = a + (int8_t)((~(*data)+1));
		p.n = res<0;
		p.z = res==0;
		p.c = ((int32_t)(a) + (int32_t)((~(*data)+1))) == res;
	}

	//CPX Compare Memory with Accumulator
	//TODO, check that I detected carry properly
	inline void CPX () {
		int8_t res = x + (int8_t)((~(*data)+1));
		p.n = res<0;
		p.z = res==0;
		p.c = ((int32_t)(x) + (int32_t)((~(*data)+1))) == res;
	}

	//CPY Compare Memory with Accumulator
	//TODO, check that I detected carry properly
	inline void CPY () {
		int8_t res = y + (int8_t)((~(*data)+1));
		p.n = res<0;
		p.z = res==0;
		p.c = ((int32_t)(y) + (int32_t)((~(*data)+1))) == res;
	}

	//DEC Decrement Memory (By 1)
	inline void DEC () {
		mem[*data]--;
		int8_t sRes = (int8_t)(mem[*data]);
		p.n = sRes < 0;
		p.z = sRes == 0;	
	}

	//INC Increment Memory (By 1)
	inline void INC () {
		mem[*data]++;
		int8_t sRes = (int8_t)(mem[*data]);
		p.n = sRes <0;
		p.z = sRes == 0;
	}

	//RTS Return from Subroutine
	inline void RTS () {
		sp++;
		uint16_t newPC = ((uint16_t)mem[sp])<<8;
		sp++;
		newPC += mem[sp]; 
		newPC++;
		pc = newPC;
	}

	//LDA Load Accumulator from Memory
	inline void LDA () {
		a = *data;
		p.n = a<0;
		p.z = a==0;
	}

	//LDX Load Index Register X from Memory
	inline void LDX () {
		x = *data;
		p.n = x<0;
		p.z = x==0;
	}

	//LDY Load Index Register Y from Memory
	inline void LDY () {
		y = *data;
		p.n = y<0;
		p.z = y==0;
	}


	//HERE LIVES BOB, ALL HAIL BOB
	int8_t bob;

	// addressing mode functions, to be called before instructions, update the global pointer data
	// accumulator mode
	void accMode() {
		data = &a;
		pc++;
	}
	// immediate mode
	void immMode() {
		bob = mem[pc+1];
		data = &bob;
		pc += 2;
	}
	// absolute mode
	void absMode() {
		uint16_t addr = (((uint16_t)mem[pc+2]) << 8) + mem[pc+1];
		data = (int8_t *) &mem[addr];
		pc += 3;
	}
	// zero-page mode
	void zrpMode() {
		uint8_t addr = mem[pc+1];
		data = (int8_t *) &mem[addr];
		pc += 2;
	}
	// absolute indexed mode using x
	void absix() {
		// TODO verify that x remains unchanged
		uint16_t addr = ((uint8_t) x) + (((uint16_t) mem[pc+2]) << 8) + mem[pc+1];
		data = (int8_t *) &mem[addr];
		pc += 3;
	}
	// absolute indexed mode using y
	void absiy() {
		// TODO verify that y remains unchanged
		uint16_t addr = ((uint8_t) y) + (((uint16_t) mem[pc+2]) << 8) + mem[pc+1];
		data = (int8_t *) &mem[addr];
		pc += 3;
	}
	// zero-paged indexed mode using x
	void zrpix() {
		// TODO verify that x remains unchanged
		uint16_t addr = ((uint8_t) x) + mem[pc+1];
		data = (int8_t *) &mem[addr];
		pc += 2;
	}
	// zero-paged indexed mode using y
	void zrpiy() {
		// TODO verify that y remains unchanged
		uint16_t addr = ((uint8_t) y) + mem[pc+1];
		data = (int8_t *) &mem[addr];
		pc += 2;
	}
	// pre-indexed indirect mode (uses x)
	void iix() {
		//x += mem[pc+1]; TODO verify x unchanged
		uint16_t addr = (((uint16_t) mem[((uint8_t) x) + mem[pc+2] + 1]) << 8) + mem[((uint8_t) x) + mem[pc+1]];
		data = (int8_t *) &mem[addr];
		pc += 3;
	}
	// post-indexed indirect mode (uses y)
	void iiy() {
		uint16_t imm = mem[pc+1];
		uint16_t addr = (((uint16_t) mem[imm+1] << 8)) + mem[imm] + ((uint8_t) y);
		data = (int8_t *) &mem[addr];
		pc += 2;
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
			
			uint8_t sics = 0;	
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
	
				//Bitwise AND with Accumulator
				case 0x29:
				{
					immMode();
					AND();
					break;	
				}
				case 0x25:
				{
					zrpMode();
					AND();
					break;	
				}
				case 0x35:
				{
					zrpix();
					AND();
					break;	
				}
				case 0x2D:
				{
					absMode();
					AND();
					break;	
				}
				case 0x3D:
				{
					absix();
					AND();
					break;	
				}
				case 0x39:
				{
					absiy();
					AND();
					break;	
				}
				case 0x21:
				{
					iix();
					AND();
					break;	
				}
				case 0x31:
				{
					iiy();
					AND();
					break;	
				}

				//Arithmetic Shift Left
				case 0x0a:
				{
					accMode();
					ASL();
					break;
				}
				case 0x06:
				{
					zrpMode();
					ASL();
					break;
				}
				case 0x16:
				{
					zrpix();
					ASL();
					break;
				}
				case 0x0E:
				{
					absMode();
					ASL();
					break;
				}
				case 0x1E:
				{
					absix();
					ASL();
					break;
				}
				
				//STA Store Accumulator in Memory
				case 0x85:
				{
					zrpMode();
					STA();
					break;
				}
				case 0x95:
				{
					zrpix();
					STA();
					break;
				}
				case 0x8d:
				{
					absMode();
					STA();
					break;
				}
				case 0x9d:
				{
					absix();
					STA();
					break;
				}
				case 0x99:
				{
					absiy();
					STA();
					break;
				}
				case 0x81:
				{
					iix();
					STA();
					break;
				}
				case 0x91:
				{
					iiy();
					STA();
					break;
				}

				//STX Store Index Register X in Memory
				case 0x86:
				{
					zrpMode();
					STX();
					break;
				}
				case 0x96:
				{
					zrpiy();
					STX();
					break;
				}
				case 0x8e:
				{
					absMode();
					STX();
					break;
				}

				//STY Store Index Register Y in Memory
				case 0x84:
				{
					zrpMode();
					STY();
					break;
				}
				case 0x94:
				{
					zrpix();
					STY();
					break;
				}
				case 0x8c:
				{
					absMode();
					STY();
					break;
				}
				
				//CMP Compare Memory With Accumulator
				case 0xc9:
				{
					immMode();
					CMP();
					break;
				}
				case 0xc5:
				{
					zrpMode();
					CMP();
					break;
				}
				case 0xd5:
				{
					zrpix();
					CMP();
					break;
				}
				case 0xcd:
				{
					absMode();
					CMP();
					break;
				}
				case 0xdd:
				{
					absix();
					CMP();
					break;
				}
				case 0xd9:
				{
					absiy();
					CMP();
					break;
				}
				case 0xc1:
				{
					iix();
					CMP();
					break;
				}
				case 0xd1:
				{
					iiy();
					CMP();
					break;
				}

				//CPX Compare Index Register X with Memory
				case 0xe0:
				{
					immMode();
					CPX();
					break;
				}
				case 0xe4:
				{
					zrpMode();
					CPX();
					break;
				}
				case 0xec:
				{
					absMode();
					CPX();
					break;
				}
				
				//CPY Compare Index Register Y with Memory
				case 0xc0:
				{
					immMode();
					CPY();
					break;
				}
				case 0xc4:
				{
					zrpMode();
					CPY();
					break;
				}
				case 0xcc:
				{
					absMode();
					CPY();
					break;
				}

				//DEC Decrement Memory (by 1)
				case 0xc6:
				{
					zrpMode();
					DEC();
					break;
				}
				case 0xd6:
				{
					zrpix();
					DEC();
					break;
				}
				case 0xce:
				{
					absMode();
					DEC();
					break;
				}
				case 0xde:
				{
					absix();
					DEC();
					break;
				}
				
				//INC Increment Memory (By 1)
				case 0xe6:
				{
					zrpMode();
					INC();
					break;
				}
				case 0xf6:
				{
					zrpix();
					INC();
					break;
				}
				case 0xee:
				{
					absMode();
					INC();
					break;
				}
				case 0xfe:
				{
					absix();
					INC();
					break;
				}

				//RTS Return from Subroutine
				case 0x60:
				{
					RTS();
					break;	
				}

				//LDA Load Accumulator from Memory
				case 0xa9:
				{
					immMode();
					LDA();
					break;
				}
				case 0xa5:
				{
					zrpMode();
					LDA();
					break;
				}
				case 0xb5:
				{
					zrpix();
					LDA();
					break;
				}
				case 0xad:
				{
					absMode();
					LDA();
					break;
				}
				case 0xbd:
				{
					absix();
					LDA();
					break;
				}
				case 0xb9:
				{
					absiy();
					LDA();
					break;
				}
				case 0xa1:
				{
					iix();
					LDA();
					break;
				}
				case 0xb1:
				{
					iiy();
					LDA();
					break;
				}

				//LDX Load Index Register X from Memory
				case 0xa2:
				{
					immMode();
					LDX();
					break;
				}
				case 0xa6:
				{
					zrpMode();
					LDX();
					break;
				}
				case 0xb6:
				{
					zrpiy();
					LDX();
					break;
				}
				case 0xae:
				{
					absMode();
					LDX();
					break;
				}
				case 0xbe:
				{
					absiy();
					LDX();
					break;
				}
				
				//LDY Load Index Register Y from Memory
				case 0xa0:
				{
					immMode();
					LDY();
					break;
				}
				case 0xa4:
				{
					zrpMode();
					LDY();
					break;
				}
				case 0xb4:
				{
					zrpix();
					LDY();
					break;
				}
				case 0xac:
				{
					absMode();
					LDY();
					break;
				}
				case 0xbc:
				{
					absix();
					LDY();
					break;
				}


				default:
				{
					printf("Encountered invalid opcode %x on line %x!\n", opcode, pc);
					dumpcore();
				} 
			}
			
			cyc += sics;	
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
	
	void power()
	{
		// TODO initialize things
	}
	
	void reset()
	{
		// TODO reset things
	}
}
