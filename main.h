#include "core.h"
#include "mem.h"
#include "flags.h"
#include "instr.h"

#include <cstdio>
#include <iostream>

namespace Core
{
	// Initialize variables
	State cpsr = {};
	uint64_t pc;
	uint64_t regs[32];

	// Only visible to Core namespace
	namespace
	{
		Block *currentBlock;
		Block blocks[MAXBLOCKSIZE];
		int nextIndex = 0;
	}
	
	// Precondition: n >= 1, 
	uint64_t ones(const uint8_t n)
	{
		if(DEBUG && n == 0)
		{
			std::cout << "Fatal error: ones function receieved n = 0" << std::endl;
			exit(1);
			return 0;
		}
		else if(n == 64)
			return 0xffffffffffffffff;
		uint64_t one = 0x7fffffffffffffff; 
		return one >> (63 - n);
	}

	uint64_t replicate64(uint64_t pattern, const uint8_t size)
	{
		uint64_t result = 0;
		uint8_t len = 0;
		while(len < 64)
		{
			result |= pattern;
			len += size;
			pattern <<= size; 
		}
		return result;
	}

	uint32_t replicate32(uint32_t pattern, const uint8_t size)
	{
		uint32_t result = 0;
		uint8_t len = 0;
		while(len < 32)
		{
			result |= pattern;
			len += size;
			pattern <<= size; 	
		}
		return result;
	}

	masks64 decode_bit_masks64(const uint8_t immn, const uint8_t imms, const uint8_t immr, const uint8_t immediate)
	{
		uint8_t imm = immn << 6 | (~imms & 0x3F);
		// Undefined:
		if(DEBUG && imm == 0)
		{
			std::cout << "ERROR: decode bitmasks got imm = 0" << std::endl;
			exit(1);
			return {};
		}
		
		uint8_t len = -1;
		while(imm > 0)
		{
			imm >>= 1;
			len++;
		}
		
		uint8_t levels = (ones(len) & 0x3F);

		uint8_t s = (imms & levels);
		uint8_t r = (immr & levels);
		uint8_t diff = ((s - r) & 0b00111111);
		uint8_t esize = 1 << len;
		uint8_t d = (diff & ones(len)) + 1;
		uint64_t welem = ones(s + 1);
		uint64_t telem = ones(d);
		uint64_t wmask = replicate64(ROR(welem, r, esize), esize);
		uint64_t tmask = replicate64(telem, esize);
		return {wmask, tmask}; 
	}

	masks32 decode_bit_masks32(const uint8_t immn, const uint8_t imms, const uint8_t immr, const uint8_t immediate)
	{
		uint8_t imm = immn << 6 | (~imms & 0x3F);
		// Undefined:
		if(DEBUG && imm == 0)
		{
			std::cout << "ERROR: decode bitmasks got imm = 0" << std::endl;
			exit(1);
			return {};
		}
		uint8_t len = -1;
		while(imm > 0)
		{
			imm >>= 1;
			len++;
		}
		
		uint8_t levels = (ones(len) & 0x3F);

		uint8_t s = (imms & levels);
		uint8_t r = (immr & levels);
		uint8_t diff = ((s - r) & 0b00111111);
		uint8_t esize = 1 << len;
		uint8_t d = (diff & ones(len)) + 1;
		uint32_t welem = (uint32_t) ones(s + 1);
		uint32_t telem = (uint32_t) ones(d);
		uint32_t wmask = replicate32(ROR(welem, r, esize), esize);
		uint32_t tmask = replicate32(telem, esize);
		return {wmask, tmask}; 
		
	}

	bool conditionHolds(const uint8_t cond)
	{
		bool result;
		bool AL = false;

		// Three most significant bits determine
		switch(cond >> 1)
		{
			case 0b000:
			{
				// EQ or NE
				result = cpsr.Z;
				break;
			}
			case 0b001:
			{
				// CS or CC
				result = cpsr.C;
				break;
			}
			case 0b010:
			{
				// MI or PL
				result = cpsr.N;
				break;
			}
			case 0b011:
			{
				// VS or VC
				result = cpsr.V;
				break;
			}
			case 0b100:
			{
				// HI or LS
				result = cpsr.C && !cpsr.Z;
				break;
			}
			case 0b101:
			{
				// GE or LT
				result = cpsr.N == cpsr.V;
				break;
			}
			case 0b110:
			{
				// GT or LE
				result = (cpsr.N == cpsr.V) && !cpsr.Z;
				break;
			}
			case 0b111:
			{
				// AL
				result = true;
				AL = true;
				break;
			}
			default:
			{
				result = false;
				std::cout << "Invalid condition in conditionHolds function" << std::endl;
				exit(1);
			}
		}
		// Least significant bit determines whether or not to invert result
		if((cond & 1) == 1 && !AL)
			return !result;
		
		return result;
	}

	// Group decode methods
	Instruction *decodeDPI(const uint32_t *instr)
	{
		if(DEBUG)
			std::cout << "Decoding DPI" << std::endl;
		
		uint8_t op0 = bitrange(instr, 24, 26);
		switch(op0)
		{
			case 0b00:
			{
				// PC-relative addressing
				uint8_t immlo = bitrange(instr, 29, 31);
				uint8_t rd = bitrange(instr, 0, 5);
				uint32_t immhi = bitrange(instr, 5, 23);
				return new InstructionADRP(pc, immlo, immhi, rd);
			}
			case 0b01:
			{
				// Add/subtract immediate with tags
				uint8_t sf = bitrange(instr, 31, 32);
				uint8_t op = bitrange(instr, 30, 31);
				uint8_t set = bitrange(instr, 29, 30);
				uint8_t shift = bitrange(instr, 22, 24);
				uint32_t imm12 = bitrange(instr, 10, 22);
				uint8_t rn = bitrange(instr, 5, 10);
				uint8_t rd = bitrange(instr, 0, 5);
				return new InstructionADDSUB(pc, imm12, sf, op, set, shift, rn, rd);
			}
			case 0b10:
			{
				uint8_t op1 = bitrange(instr, 22, 24);
				if(op1 >> 1)
				{
					// Move wide immediate
					uint8_t sf = bitrange(instr, 31, 32);
					uint8_t opc = bitrange(instr, 29, 31);
					uint8_t hw = bitrange(instr, 21, 23);
					uint8_t rd = bitrange(instr, 0, 5);
					uint32_t imm16 = bitrange(instr, 5, 21);
						
					return new InstructionMOVWIDE(pc, sf, opc, hw, rd, imm16);
				}
				else 
				{
					// Logical immediate
						
					uint8_t sf = bitrange(instr, 31, 32);
					uint8_t n = (op1 & 1);
					uint8_t immr = bitrange(instr, 16, 22);
					uint8_t imms = bitrange(instr, 10, 16);
					uint8_t rn = bitrange(instr, 5, 10);
					uint8_t rd = bitrange(instr, 0, 5);
					return new InstructionANDImm(pc, sf, n, immr, imms, rn, rd);			
				}
			}
			case 0b11:
			{
				// Bitfield 
				uint8_t sf = bitrange(instr, 31, 32);
				uint8_t immr = bitrange(instr, 16, 22);
				uint8_t imms = bitrange(instr, 10, 16);
				uint8_t rn = bitrange(instr, 5, 10);
				uint8_t rd = bitrange(instr, 0, 5);

				return new InstructionUBFM(pc, sf, immr, imms, rn, rd); 
			}
		}
		return nullptr;	
	}

	Branch *decodeB(const uint32_t *instr)
	{
		if(DEBUG)
			std::cout << "Decoding B" << std::endl;
		
		uint8_t op0 = bitrange(instr, 29, 31);
		
		switch(op0)
		{
			case 0b10:
			{
				if(bitrange(instr, 31, 32))
				{
					// Found RET: kill program
					if(DEBUG)
						std::cout << "Detected RET: killing program" << std::endl;
					exit(0);
				}
				// Conditional branch	
				const uint64_t imm = bitrange(instr, 5, 24);
				const uint8_t cond = bitrange(instr, 0, 4);
				return new CBranch(pc, imm, cond);
			}
			
			case 0b00:
			{
				// Unconditional branch
				const uint64_t imm = bitrange(instr, 0, 26);
				return new UBranch(pc, imm);
			}
			
			case 0b01:
			{
				// Compare and branch immediate: CBZ and CBNZ
				const uint8_t sf = bitrange(instr, 31, 32);
				const uint8_t op = bitrange(instr, 24, 25);
				const uint8_t rt = bitrange(instr, 0, 5);
				const uint64_t imm = bitrange(instr, 5, 24);
				return new CBZ(pc, imm, sf, op, rt); 
			}
		}
		return nullptr;
	}

	Instruction *decodeLS(const uint32_t *instr)
	{
		if(DEBUG)
			std::cout << "Decoding LS" << std::endl;
		//uint8_t op0 = bitrange(instr, 28, 32);
		//uint8_t op1 = bitrange(instr, 26, 27);
		//uint8_t op2 = bitrange(instr, 23, 25);
		//uint8_t op3 = bitrange(instr, 16, 22);
		//uint8_t op4 = bitrange(instr, 10, 12);
		uint8_t size = bitrange(instr, 30, 32);
		uint8_t rn = bitrange(instr, 5, 10);
		uint8_t rt = bitrange(instr, 0, 5);
		if(bitrange(instr, 24, 25))
		{
			// Unsigned offset
			const uint8_t opc = bitrange(instr, 22, 23);
			const uint32_t imm12 = bitrange(instr, 10, 22);
			
			if(opc)
			{
				// LDR
				switch(size)
				{
					case 0b00:
					return new InstructionLUO<uint8_t>(pc, size, rn, rt, imm12);
					case 0b10:
					return new InstructionLUO<uint32_t>(pc, size, rn, rt, imm12);
					case 0b11:
					return new InstructionLUO<uint64_t>(pc, size, rn, rt, imm12);
				}

				// Error
				return nullptr;
			}
			else
			{
				//STR
				switch(size)
				{
					case 0b00:
					return new InstructionSUO<uint8_t>(pc, size, rn, rt, imm12);
					case 0b10:
					return new InstructionSUO<uint32_t>(pc, size, rn, rt, imm12);
					case 0b11:
					return new InstructionSUO<uint64_t>(pc, size, rn, rt, imm12);
				}
				
				// Error
				return nullptr;
			}
		}
		else
		{
			// Pre-indexed LDR
			uint32_t imm9 = bitrange(instr, 12, 21);
			switch(size)
			{
				case 0b00:
				return new InstructionLPRI<uint8_t>(pc, rn, rt, imm9);
				case 0b10:
				return new InstructionLPRI<uint32_t>(pc, rn, rt, imm9);
				case 0b11:
				return new InstructionLPRI<uint64_t>(pc, rn, rt, imm9);
			}
			// Error
			return nullptr;
		}
	}
	Instruction *decodeDPR(const uint32_t *instr)
	{
		if(DEBUG)
			std::cout << "Decoding DPR" << std::endl;
		//uint8_t op0 = bitrange(instr, 30, 31);
		uint8_t op1 = bitrange(instr, 28, 29);
		uint8_t op2 = bitrange(instr, 24, 25);
		//uint8_t op2 = bitrange(instr, 21, 25);
		//uint8_t op3 = bitrange(instr, 10, 16);
		
		if(!op1 && !op2)
		{
			// Logical (shifted register)
			uint8_t sf = bitrange(instr, 31, 32);
			uint8_t shift = bitrange(instr, 22, 24);
			uint8_t n = bitrange(instr, 21, 22);
			uint8_t rm = bitrange(instr, 16, 21);
			uint8_t imm6 = bitrange(instr, 10, 16);
			uint8_t rn = bitrange(instr, 5, 10);
			uint8_t rd = bitrange(instr, 0, 5);
				
			return new InstructionORRSR(pc, sf, shift, n, rm, imm6, rn, rd);
		}
		else if(op2 && op1)
		{
			// Data-processing with 3 sources
			uint8_t sf = bitrange(instr, 31, 32);
			uint8_t rm = bitrange(instr, 16, 21);
			uint8_t ra = bitrange(instr, 10, 15);
			uint8_t rn = bitrange(instr, 5, 10);
			uint8_t rd = bitrange(instr, 0, 5);
			return new InstructionMADD(pc, sf, rm, ra, rn, rd);
		}
		else
		{
			//Add/subtract with shifted register	
			uint8_t sf = bitrange(instr, 31, 32);
			uint8_t s = bitrange(instr, 29, 30);
			uint8_t shift = bitrange(instr, 22, 24);
			uint8_t rm = bitrange(instr, 16, 21);
			uint8_t imm6 = bitrange(instr, 10, 16);
			uint8_t rn = bitrange(instr, 5, 10);
			uint8_t rd = bitrange(instr, 0, 5);
		
			return new InstructionADDSR(pc, sf, s, shift, rm, imm6, rn, rd);	
		}
	}

	// Returns a block that starts at the line provided
	// Returns nullptr if no such block exists	
	inline Block *findBlock(uint64_t start)
	{
		for(int i = 0; i < nextIndex; i++)
			if(blocks[i].getStart() == start)
				return &blocks[i];
		
		if(DEBUG)
			printf("Unable to find block starting on line %lx\n", start);
		
		return nullptr;
	}
	
	//TODO refactor/specialize to make Block invalidation more efficient	
	// Returns a block that contains the line provided
	// Returns nullptr if no such block exists	
	inline Block *findBlockContaining(uint64_t line)
	{
		for(int i = 0; i < nextIndex; i++)
			if(blocks[i].contains(line))
				return &blocks[i];
		return nullptr;
	}

	inline void prepareNewBlock()
	{
		if(DEBUG)
			std::cout << "Preparing new block..." << std::endl;
		
		if(nextIndex == MAXBLOCKSIZE)
		{
			if(DEBUG)
				std::cout << "Too many blocks: clearing them out" << std::endl;
			
			// Reset all of the blocks because we've hit the limit
			for(int i = 0; i < MAXBLOCKSIZE; i++)
				blocks[i].reset();
			
			nextIndex = 0;
		}
		
		currentBlock = &blocks[nextIndex++];
		currentBlock->setStart(pc);
	}
	
	inline uint64_t getLastBTA()
	{	
		const bool lastBranch = currentBlock->getLastBranchResult();
		Branch *branch = currentBlock->getBranch();
		return lastBranch ? branch->getTrueBranch() : branch->getFalseBranch();
	}

	inline void branchWithBlock(Block *next)
	{
		if(next != nullptr)
		{
			currentBlock = next;
			pc = currentBlock->getStart();
			
			if(DEBUG)
				printf("BRANCHED TO BLOCK STARTING AT LINE %lx\n", currentBlock->getStart());
		}
		else
		{
			pc = getLastBTA();	
			prepareNewBlock();
			
			if(DEBUG)
				printf("BRANCHED TO LINE %lx\n", pc);
		}	
	}

	void execute()
	{
		// Need to initialize the first block
		prepareNewBlock();
		
		while(true)
		{
			uint32_t encoding = mem_read<uint32_t>(pc); 
			if(DEBUG)
				printf("Reading instruction %x on line %lx\n", encoding, pc);
			
			if(!currentBlock->isComplete())
			{
				// First step: decode four important bits to segment the instructions
				// into five major categories.
				uint32_t group = bitrange(&encoding, 25, 29);
				Instruction *instr = nullptr;
				Branch *branch = nullptr;
				
				switch(group)
				{
					case 0b1000:
					case 0b1001:
					// Data processing immediate
					instr = decodeDPI(&encoding);
					break;		

					case 0b1010:
					case 0b1011:
					// Branches
					branch = decodeB(&encoding);		
					break;		

					case 0b0100:
					case 0b0110:
					case 0b1100:
					case 0b1110:
					// Loads and stores
					instr = decodeLS(&encoding);				
					break;
					
					case 0b0101:
					case 0b1101:
					// Data processing register
					instr = decodeDPR(&encoding);				
					break;
				}
					
				if(instr != nullptr) // Instruction
				{
					// Block too big: have to terminate with a fake branch
					if(currentBlock->getSize() == MAXBLOCKSIZE)
					{
						UBranch *jump = new UBranch(pc, 0);
						currentBlock->addBranch(jump);
							
						prepareNewBlock();
					}
					currentBlock->addInstruction(instr);
						
					instr->execute();
					pc += 4;
				}
				else // Branch
				{
					// This completes the Block, so we will prepare for the next block
					currentBlock->addBranch(branch);
					Block *next = currentBlock->branch();
					branchWithBlock(next);
				}
				if(DEBUG)
					dumpcore();
			}
			else
			{
				// Execute method handles setting up the next iteration
				currentBlock->execute();
			}
		}	
	}

	void step()
	{
		/*if(done)
			return;
		
		branched = false;
		
		uint32_t instr = mem_read<uint32_t>(pc); 
		if(DEBUG)
			printf("Reading instruction %x on line %lx\n", instr, pc);

		// First step: decode four important bits to segment the instructions
		// into five major categories.
		uint32_t group = bitrange(&instr, 25, 29);
		bool result = false;
		switch(group)
		{
			case 0b1000:
			case 0b1001:
			// Data processing immediate
			result = executeDPI(&instr);
			break;		

			case 0b1010:
			case 0b1011:
			// Branches
			result = executeB(&instr);		
			break;		

			case 0b0100:
			case 0b0110:
			case 0b1100:
			case 0b1110:
			// Loads and stores
			result = executeLS(&instr);				
			break;
			
			case 0b0101:
			case 0b1101:
			// Data processing register
			result = executeDPR(&instr);				
			break;
			default:
				result = false;
		}

		// Bad instruction encountered
		if(!result)
		{
			done = true;
			if(DEBUG)
				printf("BAD INSTRUCTION ON LINE %lx\n", pc);
		}

		if(!branched)
		{
			pc += 4;
		}
		else if(DEBUG)
		{
			printf("BRANCHED TO LINE %lx\n", pc);
		}*/
	}

	void dumpcore()
	{	
		for(int i = 0; i < 31; i++)
			printf("X%02d : %016lX\n", i, regs[i]);
		printf("XSP : %016lX\n", regs[31]);
	}
}

using namespace Core;

Block *Block::branch()
{
	if(DEBUG)
		printf("Branching out of block starting at line %lx\n", startLine);
	
	lastBranchResult = branchInstr->evaluate();
	
	if(lastBranchResult)
	{
		if(trueOption != nullptr)
			return trueOption;
		else
		{
			Block *next = findBlock(branchInstr->getTrueBranch());
			trueOption = next;
			return next;
		}
	}
	else
	{
		if(falseOption != nullptr)
			return falseOption;
		else
		{
			Block *next = findBlock(branchInstr->getFalseBranch());
			falseOption = next;
			return next;
		}
	}
}

void Block::execute()
{
	if(DEBUG)
	{
		printf("Executing block starting at line %lx\n", startLine);
		printf("pc = %lx", pc);
	}
	
	for(unsigned int i = 0; i < size; i++)
	{
		Instruction *instr = instrs[i];
		instr->execute();
		pc += 4;
	}
	
	Block *next = branch();
	branchWithBlock(next);	
}

void Block::clearMemory()
{
	if(DEBUG)
		printf("Clearing memory of block starting at line %lx\n", startLine);
		
	for(unsigned int i = 0; i < size; i++)
	{
		Instruction *instr = instrs[i];
		
		if(DEBUG)
			printf("Attempting to delete Instruction object %p\n", instr);
			
		delete instr;
	}

	if(DEBUG)
		printf("Finished clearing memory of block starting at line %lx\n", startLine);
}
