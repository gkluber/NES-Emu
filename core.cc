#include "core.h"
#include "flags.h"
#include "apu.h"
#include "ppu.h"
#include "util.h"

#include <cstdio>
#include <iostream>

using namespace Core;
using namespace APU;
using namespace PPU;

namespace Core
{
	// Instantiate global variables from core.h
	uint16_t pc;
	uint8_t sp;
	
	int8_t a;
	int8_t x;
	int8_t y;

	int8_t *data;
	
	uint8_t mapper;
	uint8_t prg_rom_size;
	
	uint8_t ram[2048];
	uint8_t wram[8196];
	uint8_t rom[32784];

	uint64_t cyc;
	
	Flags p;
	
	uint16_t get_effective_addr(uint16_t addr)
	{
		//if(DEBUG)
		//	printf("Getting effective address of %x...\n", addr);
		
		if(addr < 0x2000)
			addr &= 0x07ff;
		else if((addr >= 0x2000) && (addr < 0x4000))
			addr &= 0x2007;
		else if((mapper == 0) && (addr >= 0x6000) && (addr < 0x8000 ))
			addr &= 0x67ff;
		else if((mapper == 0) && (addr >= 0x8000) && (addr <= 0xffff) && prg_rom_size == 1)
			addr &= 0xbfff;
		
		//if(DEBUG)
		//	printf("Returning %x!\n", addr);
		
		return addr;
	}
		
	void mem_write(uint16_t addr, uint8_t data)
	{
		addr = get_effective_addr(addr);
		switch(addr)
		{
			// PPU registers
			// TODO set PPU latch
			case 0x2000: PPUCTRL = data; break;
			case 0x2001: PPUMASK = data; break;
			case 0x2003: OAMADDR = data; break;
			case 0x2004: OAMDATA = data; break;
			case 0x2005: PPUSCROLL = data; break;
			case 0x2006: PPUADDR = data; break;
			case 0x2007: PPUDATA = data; break;
			case 0x4014: OAMDMA = data; break;
			
			// 2A03 register map
			case 0x4000: SQ1_VOL = data; break;
			case 0x4001: SQ1_SWEEP = data; break;
			case 0x4002: SQ1_LO = data; break;
			case 0x4003: SQ1_HI = data; break;
			case 0x4004: SQ2_VOL = data; break; 
			case 0x4005: SQ2_SWEEP = data; break;
			case 0x4006: SQ2_LO = data; break;
			case 0x4007: SQ2_HI = data; break;
			case 0x4008: TRI_LINEAR = data; break;
			case 0x400a: TRI_LO = data; break;
			case 0x400b: TRI_HI = data; break;
			case 0x400c: NOISE_VOL = data; break;
			case 0x400e: NOISE_LO = data; break;
			case 0x400f: NOISE_HI = data; break;
			case 0x4010: DMC_FREQ = data; break;
			case 0x4011: DMC_RAW = data; break;
			case 0x4012: DMC_START = data; break;
			case 0x4013: DMC_LEN = data; break;
			case 0x4015: DMC_CTRL = data; break;
			case 0x4017: DMC_FCOUNTER = data; break;
			
			default:
			{
				if(addr < 0x0800)
					ram[addr] = data;
				else if((addr >= 0x6000) && (addr < 0x8000))
					wram[addr-0x6000] = data;
				else if((addr >= 0x8000) && (addr <= 0xffff))
					rom[addr-0x8000] = data;
				// else ignore the data (i.e., writing to read-only port/address)
			}
		}
	}
	
	uint8_t mem_read(uint16_t addr)
	{
		addr = get_effective_addr(addr);
		switch(addr)
		{
			//TODO PPU bus latch	
			case 0x2000: return PPUCTRL;
			case 0x2001: return PPUMASK;
			case 0x2002: return PPUSTATUS;
			case 0x2003: return OAMADDR;
			case 0x2004: return OAMDATA;
			case 0x2005: return PPUSCROLL;
			case 0x2006: return PPUADDR;
			case 0x2007: return PPUDATA;
			case 0x4014: return OAMDMA;
			
			case 0x4015: return DMC_STATUS;
			
			default:
			{
				if(addr < 0x0800)
					return ram[addr];
				else if((addr >= 0x6000) && (addr < 0x8000))
					return wram[addr-0x6000];
				else if((addr >= 0x8000) && (addr <= 0xffff))
					return rom[addr-0x8000];
				// else return 0 (writing to unmapped address)
			}
		}
		return 0;
	}
	
	uint8_t* mem_ptr(uint16_t addr)
	{
		addr = get_effective_addr(addr);
		switch(addr)
		{
			//TODO PPU bus latch	
			case 0x2000: return &PPUCTRL;
			case 0x2001: return &PPUMASK;
			case 0x2002: return &PPUSTATUS;
			case 0x2003: return &OAMADDR;
			case 0x2004: return &OAMDATA;
			case 0x2005: return &PPUSCROLL;
			case 0x2006: return &PPUADDR;
			case 0x2007: return &PPUDATA;
			case 0x4014: return &OAMDMA;
			
			case 0x4015: return &DMC_STATUS;
			default:
			{
				if(addr < 0x0800)
					return &ram[addr];
				else if((addr >= 0x6000) && (addr < 0x8000))
					return &wram[addr-0x6000];
				else if((addr >= 0x8000) && (addr <= 0xffff))
					return &rom[addr-0x8000];
				// else return 0 (writing to unmapped address)
			}
		}
		return 0; //broken address
	}
	
	///////////////////////
	// Jump Instructions //
	///////////////////////
	
	inline void JSR()
	{
		mem_write(sp, pc-1);
		sp--;
		pc = (uint16_t) mem_read(pc-1) << 8 | mem_read(pc-2);
	}
	
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
		mem_write(sp, a);
		sp--;
	}
	
	inline void PLA()
	{
		sp++;
		a = mem_read(sp);
		p.z = sp == 0;
		p.n = ((int8_t)sp) <0;
	}

	inline void PHP()
	{
		uint8_t statusReg = (p.n<<7) | (p.v<<6) | (0b11<<4) | (p.d<<3) | (p.i<<2) | (p.z<<1) | p.c;
		mem_write(sp, statusReg);
		sp--; 
	}

	inline void PLP()
	{
		sp++;
		uint8_t statusReg = mem_read(sp);
		p.n = 0 != ((((uint8_t)1)<<7) & statusReg);
		p.v = 0 != ((((uint8_t)1)<<6) & statusReg);
		p.d = 0 != ((((uint8_t)1)<<3) & statusReg);
		p.i = 0 != ((((uint8_t)1)<<2) & statusReg);
		p.z = 0 != ((((uint8_t)1)<<1) & statusReg);
		p.c = 0 != ((((uint8_t)1)) & statusReg);
	}

	//AND Bitwise AND with Accumulator
	inline void AND() {
		a &= *data;
		p.n = a<0;
		p.z = a == 0;
	}

	//ASL Arithmetic Shift Left
	//TODO sketchy write
	inline void ASL() {
		p.c = (*data)<0;
		*data = (*data) << 1;
		p.n = (*data)<0;
		p.z = (*data)==0;
	}

	//STA Store Accumulator in Memory
	inline void STA () {
		mem_write((*data), a);
	}

	//STX Store Index Register X in Memory
	inline void STX () {
		mem_write((*data), x);
	}

	//STY Store Index Register Y in Memory
	inline void STY () {
		mem_write((*data), y);
	}

	//CMP Compare Memory with Accumulator
	inline void CMP () {
		int8_t d = (int8_t)(*data);
		int8_t res = (int8_t) a - d;
		bool sign = res < 0;
		bool opsign = a < 0;
		p.n = res<0;
		p.z = res==0;
		p.c = res < max<int8_t>(a, -d);
	}

	//CPX Compare Memory with Accumulator
	//TODO, check that I detected carry properly
	inline void CPX () 
	{
		int8_t d = (int8_t)(*data);
		int8_t res = x - d;
		p.n = res < 0;
		p.z = res == 0;
		p.c = res < max<int8_t>(x, -d);
	}

	//CPY Compare Memory with Accumulator
	//TODO, check that I detected carry properly
	inline void CPY () 
	{
		int8_t d = (int8_t)(*data);
		int8_t res = y - d;
		p.n = res<0;
		p.z = res==0;
		p.c = res < max<int8_t>(y, d);
	}

	//DEC Decrement Memory (By 1)
	inline void DEC () 
	{
		mem_write((*data), mem_read(*data) -1);
		int8_t sRes = (int8_t)(mem_read(*data));
		p.n = sRes < 0;
		p.z = sRes == 0;	
	}

	//INC Increment Memory (By 1)
	inline void INC () 
	{
		mem_write((*data), mem_read(*data)+1);
		int8_t sRes = (int8_t)(mem_read(*data));
		p.n = sRes <0;
		p.z = sRes == 0;
	}

	//RTS Return from Subroutine
	inline void RTS () 
	{
		sp++;
		uint16_t newPC = ((uint16_t)mem_read(sp))<<8;
		sp++;
		newPC += mem_read(sp); 
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

	//BIT Bit Test
	inline void BIT () {
		p.n = ((int8_t)(*data))<0;
		p.v = (1 << 6) & (*data);
		uint8_t res = a & mem_read(*data);
		p.z = res == 0;
	}

	//ADC Add Memory, with Carry, to Accumulator
	inline void ADC () 
	{
		int8_t offset = *data + (int8_t) p.c;
		bool opsign = a >> 7;
		int8_t result = a + offset;
		bool sign = result >> 7; // resultant 
		p.n = result < 0;
		p.z = result == 0;
		p.c = result < max<int8_t>(a, offset);
		p.v = sign && !opsign;
		a = result;
	}
	
	inline void SBC()
	{
		int8_t offset = *data + (int8_t) p.c;
		bool opsign = a >> 7;
		int8_t result = a - offset;
		bool sign = result >> 7;
		p.n = result < 0;
		p.z = result == 0;
		p.c = result < max<int8_t>(a, -offset);
		p.v = !sign && opsign;
		a = result;
	}

	// EOR bitwise exclusive OR
	inline void EOR() {
		a ^= *data;
		p.n = a < 0;
		p.z = a == 0;
	}

	// LSR logical shift right by 1 bit
	inline void LSR() {
		p.c = *data & 1;
		*data = ((uint8_t) *data) >> 1;
		p.n = false;
		p.z = *data == 0;
	}

	// ORA bitwise OR with accumulator
	inline void ORA() {
		a |= *data;
		p.n = a < 0;
		p.z = a == 0;
	}

	// ROL rotate left shifts all bits left 1 position, carry shifted into bit 0, bit 7 goes to carry
	inline void ROL() {
		uint8_t carry = p.c ? 1 : 0;
		p.c = ((uint8_t) *data) >> 7 != 0;
		*data = (*data << 1) | carry;
		p.z = *data == 0;
		p.n = *data < 0;
	}

	// ROR rotate right, shifts all bits right 1 pos, corray shifted into bit 7, 0 goes to carry
	inline void ROR() {
		uint8_t carry = p.c ? 1 : 0;
		p.c = *data & 1 == 1;
		*data = (((uint8_t) *data) >> 1) + (carry << 7);
		p.n = *data < 0;
		p.z = *data == 0;
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
		bob = mem_read(pc+1);
		data = &bob;
		pc += 2;
	}
	// absolute mode
	void absMode() {
		uint16_t addr = (((uint16_t)mem_read(pc+2)) << 8) + mem_read(pc+1);
		data = (int8_t *) mem_ptr(addr);
		pc += 3;
	}
	// zero-page mode
	void zrpMode() {
		uint8_t addr = mem_read(pc+1);
		data = (int8_t *) mem_ptr(addr);
		pc += 2;
	}
	// absolute indexed mode using x
	void absix() {
		uint16_t addr = ((uint8_t) x) + (((uint16_t) mem_read(pc+2)) << 8) + mem_read(pc+1);
		data = (int8_t *) mem_ptr(addr);
		pc += 3;
	}
	// absolute indexed mode using y
	void absiy() {
		uint16_t addr = ((uint8_t) y) + (((uint16_t) mem_read(pc+2)) << 8) + mem_read(pc+1);
		data = (int8_t *) mem_ptr(addr);
		pc += 3;
	}
	// zero-paged indexed mode using x
	void zrpix() {
		uint16_t addr = ((uint8_t) x) + mem_read(pc+1);
		data = (int8_t *) mem_ptr(addr);
		pc += 2;
	}
	// zero-paged indexed mode using y
	void zrpiy() {
		uint16_t addr = ((uint8_t) y) + mem_read(pc+1);
		data = (int8_t *) mem_ptr(addr);
		pc += 2;
	}
	// pre-indexed indirect mode (uses x)
	void iix() {
		uint16_t addr = (((uint16_t) mem_read(((uint8_t) x) + mem_read(pc+2) + 1)) << 8) + mem_read(((uint8_t) x) + mem_read(pc+1));
		data = (int8_t *) mem_ptr(addr);
		pc += 3;
	}
	// post-indexed indirect mode (uses y)
	void iiy() {
		uint16_t imm = mem_read(pc+1);
		uint16_t addr = (((uint16_t) mem_read(imm+1) << 8)) + mem_read(imm) + ((uint8_t) y);
		data = (int8_t *) mem_ptr(addr);
		pc += 2;
	}

	void execute()
	{
		// TODO will need to accept interrupts from the system and from the PPU/APU
		// TODO will also need to work with the SDL2 GUI
		while(true)
		{
			uint8_t opcode = mem_read(pc); 
//			if(DEBUG)
//				printf("Reading instruction %x on line %lx\n", opcode, pc);
			
			uint8_t sics = 0;	
			switch(opcode)
			{
				//branch instructions
				case 0xd0:
				{
					pc += 2;
					BNE(mem_read(pc-1));
					break;
				}
				case 0xf0:
				{
					pc += 2;
					BEQ(mem_read(pc-1));
					break;
				}
				case 0x10:
				{
					pc += 2;
					BPL(mem_read(pc-1));
					break;
				}
				case 0x30:
				{
					pc += 2;
					BMI(mem_read(pc-1));
					break;
				}
				case 0x50:
				{
					pc += 2;
					BVC(mem_read(pc-1));
					break;
				}
				case 0x70:
				{
					pc += 2;
					BVS(mem_read(pc-1));
					break;
				}
				case 0x90:
				{
					pc += 2;
					BCC(mem_read(pc-1));
					break;
				}
				case 0xb0:
				{
					pc += 2;
					BCS(mem_read(pc-1));
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
					std::cout << "Warning: entered decimal mode" << std::endl;
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
				
				//BIT Bit Test
				case 0x24:
				{
					zrpMode();
					BIT();
					break;
				}
				case 0x2c:
				{
					absMode();
					BIT();
					break;
				}

				//ADC Add Memory, with Carry, to Accumulator
				case 0x69:
				{
					immMode();
					ADC();
					break;
				}
				case 0x65:
				{
					zrpMode();
					ADC();
					break;
				}
				case 0x75:
				{
					zrpix();
					ADC();
					break;
				}
				case 0x6d:
				{
					absMode();
					ADC();
					break;
				}
				case 0x7d:
				{
					absix();
					ADC();
					break;
				}
				case 0x79:
				{
					absiy();
					ADC();
					break;
				}
				case 0x61:
				{
					iix();
					ADC();
					break;
				}
				case 0x71:
				{
					iiy();
					ADC();
					break;
				}

				// SBC SuBtract with Carry
				case 0xe9:
				{
					immMode();
					SBC();
					break;
				}
				case 0xe5:
				{
					zrpMode();
					SBC();
					break;
				}
				case 0xf5:
				{
					zrpix();
					SBC();
					break;
				}
				case 0xed:
				{
					absMode();
					SBC();
					break;
				}
				case 0xfd:
				{
					absix();
					SBC();
					break;
				}
				case 0xf9:
				{
					absiy();
					SBC();
					break;
				}
				case 0xe1:
				{
					iix();
					SBC();
					break;
				}
				case 0xf1:
				{
					iiy();
					SBC();
					break;
				}
				

				// EOR bitwise exlusive OR
				case 0x49:
				{
					immMode();
					EOR();
					break;
				}
				case 0x45:
				{
					zrpMode();
					EOR();
					break;
				}
				case 0x55:
				{
					zrpix();
					EOR();
					break;
				}
				case 0x4d:
				{
					absMode();
					EOR();
					break;
				}
				case 0x5d:
				{
					absix();
					EOR();
					break;
				}
				case 0x59:
				{
					absiy();
					EOR();
					break;
				}
				case 0x41:
				{
					iix();
					EOR();
					break;
				}
				case 0x51:
				{
					iiy();
					EOR();
					break;
				}

				// LSR logical shift right shifts 1 bit right
				case 0x4a:
				{
					accMode();
					LSR();
					break;
				}
				case 0x46:
				{
					zrpMode();
					LSR();
					break;
				}
				case 0x56:
				{
					zrpix();
					LSR();
					break;
				}
				case 0x4e:
				{
					absMode();
					LSR();
					break;
				}
				case 0x5e:
				{
					absix();
					LSR();
					break;
				}

				// ORA bitwise OR with accumulator
				case 0x09:
				{
					immMode();
					ORA();
					break;
				}
				case 0x05:
				{
					zrpMode();
					ORA();
					break;
				}
				case 0x15:
				{
					zrpix();
					ORA();
					break;
				}
				case 0x0d:
				{
					absMode();
					ORA();
					break;
				}
				case 0x1d:
				{
					absix();
					ORA();
					break;
				}
				case 0x19:
				{
					absiy();
					ORA();
					break;
				}
				case 0x01:
				{
					iix();
					ORA();
					break;
				}
				case 0x11:
				{
					iiy();
					ORA();
					break;
				}

				// ROL rotate left
				case 0x2a:
				{
					accMode();
					ROL();
					break;
				}
				case 0x26:
				{
					zrpMode();
					ROL();
					break;
				}
				case 0x36:
				{
					zrpix();
					ROL();
					break;
				}
				case 0x2e:
				{
					absMode();
					ROL();
					break;
				}
				case 0x3e:
				{
					absix();
					ROL();
					break;
				}

				// ROR rotate right
				case 0x6a:
				{
					accMode();
					ROR();
					break;
				}
				case 0x66:
				{
					zrpMode();
					ROR();
					break;
				}
				case 0x76:
				{
					zrpix();
					ROR();
					break;
				}
				case 0x6e:
				{
					absMode();
					ROR();
					break;
				}
				case 0x7e:
				{
					absix();
					ROR();
					break;
				}
				case 0x4c:
				{
					// Absolute jump
					pc += 3;
					pc = (uint16_t) mem_read(pc-1) << 8 | mem_read(pc-2);
					break;
				}
				case 0x6c:
				{
					// Indirect jump
					pc += 3;
					uint16_t jmp_mem = (uint16_t) mem_read(pc-1) << 8 | mem_read(pc-2);
					pc = (uint16_t) mem_read(jmp_mem + 1) << 8 | mem_read(jmp_mem);
					break;
				}
				case 0x20:
				{
					pc += 3;
					JSR();
					break;
				}

				default:
				{
					printf("Encountered invalid opcode %x on line %x!\n", opcode, pc);
					dumpcore();
					return;
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
		if(DEBUG)
			std::cout << "Powering Core..." << std::endl;
		// Set flags
		p.n = false;
		p.v = false;
		p.d = false;
		p.i = true;
		p.z = false;
		p.c = false;
		
		a = 0;
		x = 0;
		y = 0;
		sp = 0xfd;
		mem_write(0x4017, 0x00);
		mem_write(0x4015, 0x00);
		for(uint16_t addr = 0x4000; addr <= 0x400f; addr++)
			mem_write(addr, 0x00);
		
		// Set PC to RESET vector contents
		pc = mem_read(0xfffc);
		pc |= (uint16_t) mem_read(0xfffd) << 8;	
		if(DEBUG)
			printf("pc=%x\n", pc);
	}
	
	void reset()
	{
		sp -= 3;
		p.i = true;
		mem_write(0x4015, 0x00);
	}
}
