#include "core.h"
#include "flags.h"
#include "ppu.h"

#include <iostream>
#include <fstream>

using namespace std;

bool read_ines(const char *name)
{
	ifstream rom;
	rom.open(name, ios::in | ios::binary);
	if(!rom.is_open())
	{
		std::cerr << "Could not open rom " << name << "!";
		return false;
	}
	
	char header[16];
	rom.read(header, 16);	
	if(header[0] != 'N' || header[1] != 'E' || header[2] != 'S' || header[3] != 0x1a)
	{
		std::cerr << "Rom " << name << " has invalid header file!" << std::endl;
		return false;
	}
	
	char prg_size = header[4];
	char chr_size = header[5]; // TODO check what it means if this equals zero
	char f6 = header[6];
	char f7 = header[7];
	char f8 = header[8];
	char f9 = header[9];

	// Decode flags 6
	bool mirroring = f6 & 0b1;	
	bool containsBattery = (f6 & 0b10) >> 1;
	bool hasTrainer = (f6 & 0b100) >> 2;
	bool ignoreMirroring = (f6 & 0b1000) >> 3;
	uint8_t loMapper = (f6 & 0xf0) >> 4;
	
	// Decode flags 7
	bool vs = f7 & 0b1; // VS games
	bool playchoice = (f7 & 0b10) >> 1;
	bool nes2 = (f7 & 0b1100) == 0b1000;
	uint8_t hiMapper = (f7 & 0xf0);
	
	// Decode flags 8
	// only used if containsBattery == true	
	uint8_t prg_ram_size = f8;
	
	// Decode flags 9
	bool pal = f9 & 0b1; // if true, use PAL TV system. Otherwise use NTSC.	

	// Implementing the rule of thumb stated here under section Flag 10
	// http://wiki.nesdev.com/w/index.php/INES	
	bool lastZero = (header[12] == 0) && (header[13] = 0) && (header[14] == 0) && (header[15] == 0);

	// TODO parse NES 2.0
	if(nes2)
	{
		std::cerr << "Unable to read ROM " << name << ": cannot read NES 2.0 encoding" << std::endl;
		return false;
	}
	
	uint8_t mapper;
	// See if we need to make out the upper four bits of the mapper	
	if(!lastZero && !nes2)
	{
		mapper = loMapper;
	}
	else
	{
		mapper = hiMapper | loMapper;	
	}
	
	if(mapper != 0)
	{
		std::cerr << "Mapper number " << mapper << " not supported yet" << std::endl;
		return false;
	} 
	
	Core::mapper = 0;
	
	if(DEBUG)
		printf("prg_size=%d\n", prg_size);
	Core::prg_rom_size = prg_size;
	
	if(prg_size == 0)
	{
		std::cerr << "Error: PRG size cannot be 0!" << std::endl;
		return false;
	}
	
	// Copy PRG ROM over to CPU
	int prg_bytes = 16384 * prg_size;
	rom.read((char *) Core::rom, prg_bytes);
	
	if(DEBUG)
	{
		std::cout << "Rom contents:" << std::endl;
		int max = 16384*2;
		for(int i = 0; i < 1000; i++)
			printf("$%x = %x\n", i + 0x8000, Core::rom[i]);
	}
	
	if(chr_size != 1)
	{
		std::cerr << "Error: CHR size must be 1!" << std::endl;
		return false;
	}
	int chr_bytes = 8192 * chr_size;
	for(int i = 0; i < 2; i++)
	{
		uint64_t *pattern_table = PPU::patterns[i];
		for(int j = 0; j < 512; j++)
			rom.read((char *) &pattern_table[j], 8);
		
	}
			
	rom.close();
}
