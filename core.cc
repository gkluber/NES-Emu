#include "core.h"
#include "flags.h"
#include "apu.h"
#include "ppu.h"
#include "util.h"
#include "controller.h"

#include <cstdio>
#include <iostream>

using namespace Core;
using namespace APU;
using namespace PPU;
using namespace CONTROLLER;

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

	uint8_t ppuReg = 0;
	bool memAlt = false;
	
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
		if (addr == 0x07ff) {
				printf("WHOOOOO %d\n", data);
		}
		addr = get_effective_addr(addr);
		switch(addr)
		{
			// PPU registers
			// TODO set PPU latch
			case 0x2000: 
			{
				PPUCTRL = data;
				writePPUCTRL();
				break;
			}
			case 0x2001:
			{
				PPUMASK = data;
				writePPUMASK();
				break;
			}
			case 0x2003:
			{
				OAMADDR = data;
				writeOAMADDR();
				break;
			}
			case 0x2004:
			{
				OAMDATA = data;
				writeOAMDATA();
				break;
			}
			case 0x2005:
			{
				PPUSCROLL = data;
				writePPUSCROLL();
				break;
			}
			case 0x2006:
			{
				PPUADDR = data;
				writePPUADDR();
				break;
			}
			case 0x2007:
			{
				PPUDATA = data;
				writePPUDATA();
				break;
			}
			case 0x4014:
			{
				OAMDMA = data;
				writeOAMDMA();
				break;
			}
			
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
			case 0x4016: set_controller_strobe((data & 1) == 1); break;
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
			case 0x2000: ppuReg = 1; return &PPUCTRL;
			case 0x2001: ppuReg = 2; return &PPUMASK;
			case 0x2002: ppuReg = 3; return &PPUSTATUS;
			case 0x2003: ppuReg = 4; return &OAMADDR;
			case 0x2004: ppuReg = 5; return &OAMDATA;
			case 0x2005: ppuReg = 6; return &PPUSCROLL;
			case 0x2006: ppuReg = 7; return &PPUADDR;
			case 0x2007: ppuReg = 8; return &PPUDATA;
			case 0x4014: ppuReg = 9; return &OAMDMA;
			
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
		uint16_t pcToSave = pc-1;
		uint8_t hiBite = pcToSave >> 8;
		uint8_t loBite = (pcToSave << 8) >> 8;
		mem_write(sp, hiBite);
		sp--;
		mem_write(sp, loBite);
		sp--;
		pc = ( (uint16_t)(mem_read(pc-1)) << 8) | (uint16_t)(mem_read(pc-2));
	}
	
	/////////////////////////
	// Branch instructions //
	/////////////////////////
		
	inline bool BPL(int8_t d)
	{
		if(!p.n)
			pc += d;	
		return !p.n;
	}
	
	inline bool BMI(int8_t d)
	{
		if(p.n)
			pc += d;	
		return p.n;
	}

	inline bool BVC(int8_t d)
	{
		if(!p.v)
			pc += d;	
		return !p.v;
	}
	
	inline bool BVS(int8_t d)
	{
		if(p.v)
			pc += d;	
		return p.v;
	}
	
	inline bool BCC(int8_t d)
	{
		if(!p.c)
			pc += d;	
		return !p.c;
	}
	
	inline bool BCS(int8_t d)
	{
		if (p.c)
			pc += d;	
		return p.c;
	}
	
	inline bool BNE(int8_t d)
	{
		if(!p.z)
			pc += d;
		return !p.z;
	}
	
	inline bool BEQ(int8_t d)
	{
		if(p.z)
			pc += d;
		return p.z;
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
		p.z = a == 0;
		p.n = a <0;
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
		memAlt = true;
	}

	//STA Store Accumulator in Memory
	//TODO modded from a write call mem_write(*data, a);
	inline void STA () {
		*data = a;
		memAlt = true;
	}

	//STX Store Index Register X in Memory
	inline void STX () {
		*data = x;
		memAlt = true;
	}

	//STY Store Index Register Y in Memory
	inline void STY () {
		*data = y;
		memAlt = true;
	}
	//old line is p.c = res < max<int8_t>(a, -d)
	//CMP Compare Memory with Accumulator

	inline void Old_CMP() {

		int8_t offset = *data + (int8_t)(1-1);
		bool opsign = a >> 7;
		int8_t result = a - offset;
		bool sign = result >> 7;
		p.n = result < 0;
		p.z = result == 0;
		p.c = result >= 0;
	}

	inline void CMP () {
		int8_t d = (int8_t)(*data);
		int8_t res = (int8_t) a - d;
		bool sign = res < 0;
		bool opsign = a < 0;
		p.n = res<0;
		p.z = res==0;
//		p.c = ((uint32_t)((uint8_t)a) + (uint32_t)((~*data)) + 1) > 255;
		p.c = (uint8_t)a >= (uint8_t)d;
	}


	//CPX Compare Memory with Accumulator
	//TODO, check that I detected carry properly
	inline void CPX () 
	{
		int8_t d = (int8_t)(*data);
		int8_t res = x - d;
		p.n = res < 0;
		p.z = res == 0;
		p.c = (uint8_t)x >= (uint8_t)d;
	}

	//CPY Compare Memory with Accumulator
	//TODO, check that I detected carry properly
	inline void CPY () 
	{
		int8_t d = (int8_t)(*data);
		int8_t res = y - d;
		p.n = res<0;
		p.z = res==0;
		p.c = (uint8_t)y >= (uint8_t)d;
	}

	//DEC Decrement Memory (By 1)
	//TODO changed
	inline void DEC () 
	{
		(*data)--;
//		mem_write((*data), mem_read(*data) -1);
		int8_t sRes = (int8_t)(*data);
		p.n = sRes < 0;
		p.z = sRes == 0;	
	}

	//INC Increment Memory (By 1)
	//TODO changed
	inline void INC () 
	{
		(*data)++;
//		mem_write((*data), mem_read(*data)+1);
		int8_t sRes = (int8_t)(*data);
		p.n = sRes <0;
		p.z = sRes == 0;
	}

	//RTS Return from Subroutine
	inline void RTS () 
	{
		sp++;
		uint16_t newPC = ((uint16_t)mem_read(sp));
		sp++;
		newPC |= ((uint16_t)(mem_read(sp)) <<8); 
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
		printf("here %x addr of %x \n", mem_read(*data), *data); //TODO
		p.n = ((int8_t)(*data))<0;
		p.v = (1 << 6) & (*data);
		uint8_t res = a & (*data);
		p.z = res == 0;
	}

	//ADC Add Memory, with Carry, to Accumulator
	inline void ADC () 
	{
		int8_t offset = *data + (uint8_t) p.c;
		bool opsign = a >> 7;
		int8_t result = a + offset;
		bool sign = result >> 7; // resultant 
		p.n = result < 0;
		p.z = result == 0;
		printf("test val %x %x %x\n" ,(uint32_t)p.c, (uint32_t)((uint8_t)(*data)), ((uint32_t)((uint8_t)a)));
		p.c = ((uint32_t)p.c + (uint32_t)((uint8_t)(*data)) + (uint32_t)a) > 255;
		int32_t testSum = (int32_t)(p.c + (uint32_t)(*data) + a);
		p.v = testSum >127 || testSum<-128;
		a = result;
	}
	inline void old_ADC () 
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
	
	//pot fix added TODO
	inline void SBC()
	{
		int8_t offset = *data + (int8_t)(1- p.c);
		bool opsign = a >> 7;
		int8_t result = a - offset;
		bool sign = result >> 7;
		p.n = result < 0;
		p.z = result == 0;
		p.c = result >= 0;
		p.v = !sign && opsign;
		a = result;
	}
	inline void old_SBC()
	{
		int8_t offset = *data + (int8_t)(1- p.c);
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
		memAlt = true;
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
		memAlt = true;
	}

	// ROR rotate right, shifts all bits right 1 pos, corray shifted into bit 7, 0 goes to carry
	inline void ROR() {
		uint8_t carry = p.c ? 1 : 0;
		p.c = *data & 1 == 1;
		*data = (((uint8_t) *data) >> 1) + (carry << 7);
		p.n = *data < 0;
		p.z = *data == 0;
		memAlt = true;
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
		uint16_t addr = (((uint16_t)mem_read(pc+2)) << 8) | mem_read(pc+1);
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
	bool absix() {
		uint16_t addr = ((uint8_t) x) + (((uint16_t) mem_read(pc+2)) << 8) + mem_read(pc+1);
		data = (int8_t *) mem_ptr(addr);
		pc += 3;
		return ((addr) & 0xFF00) != (((((uint16_t) mem_read(pc-1))<<8)+ mem_read(pc-2)) & 0xFF00);
	}
	// absolute indexed mode using y
	bool absiy() {
		uint16_t addr = ((uint8_t) y) + (((uint16_t) mem_read(pc+2)) << 8) + mem_read(pc+1);
		data = (int8_t *) mem_ptr(addr);
		pc += 3;
		return ((addr) & 0xFF00) != (((((uint16_t) mem_read(pc-1))<<8)+ mem_read(pc-2)) & 0xFF00);
	}
	// zero-paged indexed mode using x
	void zrpix() {
		uint8_t addr = ((uint8_t) x) + mem_read(pc+1);
		data = (int8_t *) mem_ptr(addr);
		pc += 2;
	}
	// zero-paged indexed mode using y
	void zrpiy() {
		uint8_t addr = ((uint8_t) y) + mem_read(pc+1);
		data = (int8_t *) mem_ptr(addr);
		pc += 2;
	}
	// pre-indexed indirect mode (uses x)
	void iix() {
		//printf("value at 0x200 %x\n", mem_read(200));
		uint8_t interAddr = (uint8_t) x + (uint8_t)mem_read(pc+1);
		//printf("inter addr %x\n", interAddr);
		uint16_t addr = ((uint16_t)mem_read((uint8_t)(interAddr + 1)) << 8) + (uint16_t)mem_read(interAddr);
		//printf("addr is %x\n", addr);
		data = (int8_t *) mem_ptr(addr);
		//printf("finally data %x\n", *data);
		pc += 2;
	}
	// post-indexed indirect mode (uses y)
	bool iiy() {
		//printf("val at 0x00ff is %x at 0x0000 is %x\n", mem_read(0x00ff), mem_read(0x0000));
		uint8_t imm = (uint8_t)(mem_read(pc+1));
		//printf("imm is %x and imm+1 is %x\n", imm, (uint8_t)(imm+1));
		uint16_t addr = ((((uint16_t) (uint8_t)(mem_read((uint8_t)(imm+1)))) << 8)) + mem_read(imm) + ((uint8_t) y);
		//printf("then addr is %x which is %x + %x + %x\n", addr, (uint16_t) (uint8_t)(mem_read((uint8_t)(imm+1))), mem_read(imm),(uint8_t) y);
		data = (int8_t *) mem_ptr(addr);
		//printf("and data is %x\n", *data);
		pc += 2;
		return ((addr) & 0xFF00) != (((((uint16_t) mem_read((uint8_t)(imm+1)))<<8)+ mem_read(imm)) & 0xFF00);
	}
	//for branch instructions, call after updating pc normally
	bool branchPageCross (int8_t offset) {		
		return ((pc) & 0xFF00) != ((pc+offset) & 0xFF00);
	}

	void execute()
	{
		// TODO will need to accept interrupts from the system and from the PPU/APU
		// TODO will also need to work with the SDL2 GUI
		uint32_t cc = 0;
		//while(true)
		for (int i = 0; i < 50000; i++)
			step();
	}
	
	int step()
	{
		// Initialize for NESTEST
		static bool initialized = false;
		static uint32_t cc;
		if(!initialized)
		{
			initialized = true;
			cc = 0;
		}
		
		uint8_t opcode = mem_read(pc);
		if(DEBUG) {
			printf("Reading instruction %x on line %lx\n", opcode, pc);
			if(NESTEST) {
				dumpcore();
				cc++;
				if (cc%10 == 0) {
					getchar();
				}
			}
//				scanf("%c", &c);
		}
		
		uint8_t sics = 0;	
		bool branched = false;
		bool brPC = false;
		
		// Reset cycle count
		cyc = 0;
		
		switch(opcode)
		{
			//branch instructions
			case 0xd0:
			{
				pc += 2;
				cyc += 2;
				brPC = branchPageCross(mem_read(pc-1));
				branched = BNE(mem_read(pc-1));
				if (branched) {
					cyc++;
					cyc += brPC ? 1 : 0;
				}
				break;
			}
			case 0xf0:
			{
				pc += 2;
				cyc += 2;	
				brPC = branchPageCross(mem_read(pc-1));
				branched = BEQ(mem_read(pc-1));
				if (branched) {
					cyc++;
					cyc += brPC ? 1 : 0;
				}
				break;
			}
			case 0x10:
			{
				pc += 2;
				cyc += 2;
				brPC = branchPageCross(mem_read(pc-1));
				branched = BPL(mem_read(pc-1));
				if (branched) {
					cyc++;
					cyc += brPC ? 1 : 0;
				}
				break;
			}
			case 0x30:
			{
				pc += 2;
				cyc += 2;
				brPC = branchPageCross(mem_read(pc-1));
				branched = BMI(mem_read(pc-1));
				if (branched) {
					cyc++;
					cyc += brPC ? 1 : 0;
				}
				break;
			}
			case 0x50:
			{
				pc += 2;
				cyc += 2;
				brPC = branchPageCross(mem_read(pc-1));
				branched = BVC(mem_read(pc-1));
				if (branched) {
					cyc++;
					cyc += brPC ? 1 : 0;
				}
				break;
			}
			case 0x70:
			{
				pc += 2;
				cyc += 2;
				brPC = branchPageCross(mem_read(pc-1));
				branched = BVS(mem_read(pc-1));
				if (branched) {
					cyc++;
					cyc += brPC ? 1 : 0;
				}
				break;
			}
			case 0x90:
			{
				pc += 2;
				cyc += 2;
				brPC = branchPageCross(mem_read(pc-1));
				branched = BCC(mem_read(pc-1));
				if (branched) {
					cyc++;
					cyc += brPC ? 1 : 0;
				}
				break;
			}
			case 0xb0:
			{
				pc += 2;
				cyc += 2;
				brPC = branchPageCross(mem_read(pc-1));
				branched = BCS(mem_read(pc-1));
				if (branched) {
					cyc++;
					cyc += brPC ? 1 : 0;
				}
				break;
			}

			// register instructions
			case 0xaa:
			{
				pc++;
				TAX();
				cyc+=2;
				break;
			}
			case 0x8a:
			{
				pc++;
				TXA();
				cyc+=2;
				break;
			}
			case 0xca:
			{
				pc++;
				DEX();
				cyc+=2;
				break;
			}
			case 0xe8:
			{
				pc++;
				INX();
				cyc+=2;
				break;
			}
			case 0xa8:
			{
				pc++;
				TAY();
				cyc+=2;
				break;
			}
			case 0x98:
			{
				pc++;
				TYA();
				cyc+=2;
				break;
			}
			case 0x88:
			{
				pc++;
				DEY();
				cyc+=2;
				break;
			}

			case 0xc8:
			{
				pc++;
				INY();
				cyc+=2;
				break;
			}
			

			//NOP
			case 0xea:
			{
				pc++;	
				cyc+=2;
				break;
			}
			
			//Flag (Processor Status) Instructions
			case 0x18:
			{
				pc++;
				CLC();
				cyc+=2;
				break;
			}	
			case 0x38:
			{
				pc++;
				SEC();
				cyc+=2;
				break;
			}
			case 0x58:
			{
				pc++;
				CLI();
				cyc+=2;
				break;
			}
			case 0x78:
			{
				pc++;
				SEI();
				cyc+=2;
				break;
			}
			case 0xb8:
			{
				pc++;
				CLV();
				cyc+=2;
				break;
			}
			case 0xd8:
			{
				pc++;
				CLD();
				cyc+=2;
				break;
			}
			case 0xf8:
			{
				pc++;
				SED();
				cyc+=2;
				std::cout << "Warning: entered decimal mode" << std::endl;
				break;
			}

			//Stack Instructions
			case 0x9a:
			{
				pc++;
				TXS();
				cyc+=2;
				break;
			}
			case 0xba:
			{
				pc++;
				TSX();
				cyc+=2;
				break;
			}
			case 0x48:
			{
				pc++;
				PHA();
				cyc+=3;
				break;
			}
			case 0x68:
			{
				pc++;
				PLA();
				cyc+=4;
				break;
			}
			case 0x08:
			{
				pc++;
				PHP();
				cyc+=3;
				break;
			}
			case 0x28:
			{
				pc++;
				PLP();
				cyc+=4;
				break;
			}

			//Bitwise AND with Accumulator
			case 0x29:
			{
				immMode();
				AND();
				cyc+=2;
				break;	
			}
			case 0x25:
			{
				zrpMode();
				AND();
				cyc+=3;
				break;	
			}
			case 0x35:
			{
				zrpix();
				AND();
				cyc+=4;
				break;	
			}
			case 0x2D:
			{
				absMode();
				AND();
				cyc+=4;
				break;	
			}
			case 0x3D:
			{
				cyc += absix() ? 1 : 0;
				AND();
				cyc+=4;
				break;	
			}
			case 0x39:
			{
				cyc += absiy() ? 1 : 0;
				AND();
				cyc += 4;
				break;	
			}
			case 0x21:
			{
				iix();
				AND();
				cyc+=6;
				break;	
			}
			case 0x31:
			{
				cyc += iiy() ? 1 : 0;
				AND();
				cyc += 5;
				break;	
			}

			//Arithmetic Shift Left
			case 0x0a:
			{
				accMode();
				ASL();
				cyc+=2;
				break;
			}
			case 0x06:
			{
				zrpMode();
				ASL();
				cyc+=5;
				break;
			}
			case 0x16:
			{
				zrpix();
				ASL();
				cyc+=6;
				break;
			}
			case 0x0E:
			{
				absMode();
				ASL();
				cyc+=6;
				break;
			}
			case 0x1E:
			{
				absix();
				ASL();
				cyc+=7;
				break;
			}
			
			//STA Store Accumulator in Memory
			case 0x85:
			{
				zrpMode();
				STA();
				cyc+=3;
				break;
			}
			case 0x95:
			{
				zrpix();
				STA();
				cyc+=4;
				break;
			}
			case 0x8d:
			{
				absMode();
				STA();
				cyc+=4;
				break;
			}
			case 0x9d:
			{
				absix();
				STA();
				cyc+=5;
				break;
			}
			case 0x99:
			{
				absiy();
				STA();
				cyc+=5;
				break;
			}
			case 0x81:
			{
				iix();
				STA();
				cyc+=6;
				break;
			}
			case 0x91:
			{
				iiy();
				STA();
				cyc+=6;
				break;
			}

			//STX Store Index Register X in Memory
			case 0x86:
			{
				zrpMode();
				STX();
				cyc+=3;
				break;
			}
			case 0x96:
			{
				zrpiy();
				STX();
				cyc+=4;
				break;
			}
			case 0x8e:
			{
				absMode();
				STX();
				cyc+=4;
				break;
			}

			//STY Store Index Register Y in Memory
			case 0x84:
			{
				zrpMode();
				STY();
				cyc+=3;
				break;
			}
			case 0x94:
			{
				zrpix();
				STY();
				cyc+=4;
				break;
			}
			case 0x8c:
			{
				absMode();
				STY();
				cyc+=4;
				break;
			}
			
			//CMP Compare Memory With Accumulator
			case 0xc9:
			{
				immMode();
				CMP();
				cyc+=2;
				break;
			}
			case 0xc5:
			{
				zrpMode();
				CMP();
				cyc+=3;
				break;
			}
			case 0xd5:
			{
				zrpix();
				CMP();
				cyc+=4;
				break;
			}
			case 0xcd:
			{
				absMode();
				CMP();
				cyc+=4;
				break;
			}
			case 0xdd:
			{
				cyc += absix() ? 1 : 0;
				CMP();
				cyc+=4;
				break;
			}
			case 0xd9:
			{
				cyc += absiy() ? 1 : 0;
				CMP();
				cyc += 4;
				break;
			}
			case 0xc1:
			{
				iix();
				CMP();
				cyc+=6;
				break;
			}
			case 0xd1:
			{
				cyc += iiy() ? 1 : 0;
				CMP();
				cyc += 5;
				break;
			}

			//CPX Compare Index Register X with Memory
			case 0xe0:
			{
				immMode();
				CPX();
				cyc+=2;
				break;
			}
			case 0xe4:
			{
				zrpMode();
				CPX();
				cyc+=3;
				break;
			}
			case 0xec:
			{
				absMode();
				CPX();
				cyc+=4;
				break;
			}
			
			//CPY Compare Index Register Y with Memory
			case 0xc0:
			{
				immMode();
				CPY();
				cyc+=2;
				break;
			}
			case 0xc4:
			{
				zrpMode();
				CPY();
				cyc+=3;
				break;
			}
			case 0xcc:
			{
				absMode();
				CPY();
				cyc+=4;
				break;
			}

			//DEC Decrement Memory (by 1)
			case 0xc6:
			{
				zrpMode();
				DEC();
				cyc+=5;
				break;
			}
			case 0xd6:
			{
				zrpix();
				DEC();
				cyc+=6;
				break;
			}
			case 0xce:
			{
				absMode();
				DEC();
				cyc+=6;
				break;
			}
			case 0xde:
			{
				absix();
				DEC();
				cyc+=7;
				break;
			}
			
			//INC Increment Memory (By 1)
			case 0xe6:
			{
				zrpMode();
				INC();
				cyc+=5;
				break;
			}
			case 0xf6:
			{
				zrpix();
				INC();
				cyc+=6;
				break;
			}
			case 0xee:
			{
				absMode();
				INC();
				cyc+=6;
				break;
			}
			case 0xfe:
			{
				absix();
				INC();
				cyc+=7;
				break;
			}

			//RTS Return from Subroutine
			case 0x60:
			{
				RTS();
				cyc+=6;
				break;	
			}

			//LDA Load Accumulator from Memory
			case 0xa9:
			{
				immMode();
				LDA();
				cyc+=2;
				break;
			}
			case 0xa5:
			{
				zrpMode();
				LDA();
				cyc+=3;
				break;
			}
			case 0xb5:
			{
				zrpix();
				LDA();
				cyc+=4;
				break;
			}
			case 0xad:
			{
				absMode();
				LDA();
				cyc+=4;
				break;
			}
			case 0xbd:
			{
				cyc += absix() ? 1 : 0;
				LDA();
				cyc += 4;
				break;
			}
			case 0xb9:
			{
				cyc += absiy() ? 1 : 0;
				LDA();
				cyc += 4;
				break;
			}
			case 0xa1:
			{
				iix();
				LDA();
				cyc+=6;
				break;
			}
			case 0xb1:
			{
				cyc += iiy() ? 1 : 0;
				LDA();
				cyc += 5;
				break;
			}

			//LDX Load Index Register X from Memory
			case 0xa2:
			{
				immMode();
				LDX();
				cyc+=2;
				break;
			}
			case 0xa6:
			{
				zrpMode();
				LDX();
				cyc+=3;
				break;
			}
			case 0xb6:
			{
				zrpiy();
				LDX();
				cyc+=4;
				break;
			}
			case 0xae:
			{
				absMode();
				LDX();
				cyc+=4;
				break;
			}
			case 0xbe:
			{
				cyc += absiy() ? 1 : 0;
				LDX();
				cyc += 4;
				break;
			}
			
			//LDY Load Index Register Y from Memory
			case 0xa0:
			{
				immMode();
				LDY();
				cyc+=2;
				break;
			}
			case 0xa4:
			{
				zrpMode();
				LDY();
				cyc+=3;
				break;
			}
			case 0xb4:
			{
				zrpix();
				LDY();
				cyc+=4;
				break;
			}
			case 0xac:
			{
				absMode();
				LDY();
				cyc+=4;
				break;
			}
			case 0xbc:
			{
				cyc += absix() ? 1 : 0;
				LDY();
				cyc += 4;
				break;
			}
			
			//BIT Bit Test
			case 0x24:
			{
				zrpMode();
				BIT();
				cyc+=3;
				break;
			}
			case 0x2c:
			{
				absMode();
				BIT();
				cyc+=4;
				break;
			}

			//ADC Add Memory, with Carry, to Accumulator
			case 0x69:
			{
				immMode();
				ADC();
				cyc+=2;
				break;
			}
			case 0x65:
			{
				zrpMode();
				ADC();
				cyc+=3;
				break;
			}
			case 0x75:
			{
				zrpix();
				ADC();
				cyc+=4;
				break;
			}
			case 0x6d:
			{
				absMode();
				ADC();
				cyc+=4;
				break;
			}
			case 0x7d:
			{
				cyc += absix() ? 1 : 0;
				ADC();
				cyc += 4;
				break;
			}
			case 0x79:
			{
				cyc += absiy() ? 1 : 0 ;
				ADC();
				cyc += 4;
				break;
			}
			case 0x61:
			{
				iix();
				ADC();
				cyc+=6;
				break;
			}
			case 0x71:
			{
				cyc += iiy() ? 1 : 0;
				ADC();
				cyc += 5;
				break;
			}

			// SBC SuBtract with Carry
			case 0xe9:
			{
				immMode();
				SBC();
				cyc+=2;
				break;
			}
			case 0xe5:
			{
				zrpMode();
				SBC();
				cyc+=3;
				break;
			}
			case 0xf5:
			{
				zrpix();
				SBC();
				cyc+=4;
				break;
			}
			case 0xed:
			{
				absMode();
				SBC();
				cyc+=4;
				break;
			}
			case 0xfd:
			{
				cyc += absix() ? 1 : 0;
				SBC();
				cyc += 4;
				break;
			}
			case 0xf9:
			{
				cyc += absiy() ? 1 : 0;
				SBC();
				cyc += 4;
				break;
			}
			case 0xe1:
			{
				iix();
				SBC();
				cyc+=6;
				break;
			}
			case 0xf1:
			{
				cyc += iiy() ? 1 : 0;
				SBC();
				cyc += 5;
				break;
			}
			

			// EOR bitwise exlusive OR
			case 0x49:
			{
				immMode();
				EOR();
				cyc+=2;
				break;
			}
			case 0x45:
			{
				zrpMode();
				EOR();
				cyc+=3;
				break;
			}
			case 0x55:
			{
				zrpix();
				EOR();
				cyc+=4;
				break;
			}
			case 0x4d:
			{
				absMode();
				EOR();
				cyc+=4;
				break;
			}
			case 0x5d:
			{
				cyc += absix() ? 1 : 0;
				EOR();
				cyc += 4;
				break;
			}
			case 0x59:
			{
				cyc += absiy() ? 1 : 0;
				EOR();
				cyc += 4;
				break;
			}
			case 0x41:
			{
				iix();
				EOR();
				cyc+=6;
				break;
			}
			case 0x51:
			{
				cyc += iiy() ? 1 : 0;
				EOR();
				cyc += 5;
				break;
			}

			// LSR logical shift right shifts 1 bit right
			case 0x4a:
			{
				accMode();
				LSR();
				cyc+=2;
				break;
			}
			case 0x46:
			{
				zrpMode();
				LSR();
				cyc+=5;
				break;
			}
			case 0x56:
			{
				zrpix();
				LSR();
				cyc+=6;
				break;
			}
			case 0x4e:
			{
				absMode();
				LSR();
				cyc+=6;
				break;
			}
			case 0x5e:
			{
				absix();
				LSR();
				cyc+=7;
				break;
			}

			// ORA bitwise OR with accumulator
			case 0x09:
			{
				immMode();
				ORA();
				cyc+=2;
				break;
			}
			case 0x05:
			{
				zrpMode();
				ORA();
				cyc+=3;
				break;
			}
			case 0x15:
			{
				zrpix();
				ORA();
				cyc+=4;
				break;
			}
			case 0x0d:
			{
				absMode();
				ORA();
				cyc+=4;
				break;
			}
			case 0x1d:
			{
				cyc += absix() ? 1 : 0;
				ORA();
				cyc += 4;
				break;
			}
			case 0x19:
			{
				cyc += absiy() ? 1 : 0;
				ORA();
				cyc += 4;
				break;
			}
			case 0x01:
			{
				iix();
				ORA();
				cyc+=6;
				break;
			}
			case 0x11:
			{
				cyc += iiy() ? 1 : 0;
				ORA();
				cyc += 5;
				break;
			}

			// ROL rotate left
			case 0x2a:
			{
				accMode();
				ROL();
				cyc+=2;
				break;
			}
			case 0x26:
			{
				zrpMode();
				ROL();
				cyc+=5;
				break;
			}
			case 0x36:
			{
				zrpix();
				ROL();
				cyc+=6;
				break;
			}
			case 0x2e:
			{
				absMode();
				ROL();
				cyc+=6;
				break;
			}
			case 0x3e:
			{
				absix();
				ROL();
				cyc+=7;
				break;
			}

			// ROR rotate right
			case 0x6a:
			{
				accMode();
				ROR();
				cyc+=2;
				break;
			}
			case 0x66:
			{
				zrpMode();
				ROR();
				cyc+=5;
				break;
			}
			case 0x76:
			{
				zrpix();
				ROR();
				cyc+=6;
				break;
			}
			case 0x6e:
			{
				absMode();
				ROR();
				cyc+=6;
				break;
			}
			case 0x7e:
			{
				absix();
				ROR();
				cyc+=7;
				break;
			}
			case 0x4c:
			{
				// Absolute jump
				pc += 3;
				pc = (uint16_t) mem_read(pc-1) << 8 | mem_read(pc-2);
				cyc+=3;
				break;
			}
			case 0x6c:
			{
				// Indirect jump
				pc += 3;
				uint16_t jmp_mem = (uint16_t) mem_read(pc-1) << 8 | mem_read(pc-2);
				pc = (uint16_t) mem_read(jmp_mem + 1) << 8 | mem_read(jmp_mem);
				cyc+=5;
				break;
			}
			case 0x20:
			{
				pc += 3;
				JSR();
				cyc+=6;
				break;
			}

			//BRK
/*				case 0x00:
			{
				pc += 2;
				mem_write(sp,(uint8_t)( pc&0xff00));
				sp--;
				mem_write(sp, (uint8_t)(pc&0x00ff));
				sp--;
				uint8_t statusReg = (p.n<<7) | (p.v<<6)| (1<<5) | (p.d<<3) | (p.i<<2) | (p.z<<1) | p.c;
			}
*/
			//RTI
			case 0x40:
			{
				sp++;
				uint8_t newSR = mem_read(sp);
				sp++;
				uint16_t newPC = (uint8_t)(mem_read(sp));
				sp++;
				newPC += ((uint16_t)((uint8_t)(mem_read(sp)))) << 8;
				p.n = newSR & 0x80;
				p.v = newSR & 0x40;
				p.d = newSR & 0x08;
				p.i = newSR & 0x04;
				p.z = newSR & 0x02;
				p.c = newSR & 0x01;
				pc = newPC;
				cyc += 6;
				break;
			}

			default:
			{
				printf("Encountered invalid opcode %x on line %x!\n", opcode, pc);
				dumpcore();
			} 
		}
		if (ppuReg > 0 && memAlt) {
			switch (ppuReg) {
				case 1: writePPUCTRL(); break;
				case 2: writePPUMASK(); break;
				//case 3: writePPUSTATUS(); break;
				case 4: writeOAMADDR(); break;
				case 5: writeOAMDATA(); break;
				case 6: writePPUSCROLL(); break;
				case 7: writePPUADDR(); break;
				case 8: writePPUDATA(); break;
				case 9: writeOAMDMA(); break;
			}
		}
		ppuReg = 0;
		memAlt = false;
//		cyc += sics;	
		return cyc;
	}

	void dumpcore()
	{
		printf("Core dump:\n");
		printf("PC: %x\n", pc);
		printf("SP: %x\n", sp);
		printf("A: %x\n", (uint8_t) a);
		printf("X: %x\n", (uint8_t) x);
		printf("Y: %x\n", (uint8_t) y);
		//printf("Flags: N=%d, V=%d, D=%d, I=%d, Z=%d, C=%d\n", p.n, p.v, p.d, p.i, p.z, p.c);
		printf("Cycles: %llu\n", cyc+7);
		uint8_t statusReg = (p.n<<7) | (p.v<<6)| (1<<5) | (p.d<<3) | (p.i<<2) | (p.z<<1) | p.c;
		printf("Flag byte P:  %x\n", statusReg);
/*
		for(int i = 0; i < 0x1000; i++)
		{
			printf("%02x ", mem_read(i));
			if((i % 8 == 0) && (i != 0))
				std::cout << std::endl;
		}
		std::cout << std::endl;
*/		
		if  (NESTEST) {
			printf("02h = %x and 03h = %x\n", mem_read(2), mem_read(3));
		}
		//uint16_t loc = 0x6004;
//		printf("Now to check for correctness...\n");
//		while (mem_read(loc) != 0) {
//			printf("! %d ", mem_read(loc));		
//		}
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
		cyc = 0;
		sp = 0xfd;
		mem_write(0x4017, 0x00);
		mem_write(0x4015, 0x00);
		for(uint16_t addr = 0x4000; addr <= 0x400f; addr++)
			mem_write(addr, 0x00);
		
		// Set PC to RESET vector contents
		pc = mem_read(0xfffc);
		pc |= (uint16_t) mem_read(0xfffd) << 8;
		
		if (NESTEST) {
				pc = 0xc000;
				ram[0x0180] = 0x33;
				ram[0x017f] = 0x69;

		}
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
