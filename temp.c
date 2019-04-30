
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
		data = &mem[addr];
	}
	// zero-page mode
	void zrpMode() {
		uint8_t addr = mem[pc+1];
		data = &mem[addr];
	}
	// absolute indexed mode using x
	void absix() {
		// TODO verify that x remains unchanged
		uint16_t addr = ((uint8_t x) + (mem[pc+2] << 8) + mem[pc+1];
		data = &mem[addr];
	}
	// absolute indexed mode using y
	void absiy() {
		// TODO verify that y remains unchanged
		uint16_t addr = ((uint8_t) y) + (mem[pc+2] << 8) + mem[pc+1];
		data = &mem[addr];
	}
	// zero-paged indexed mode using x
	void zrpix() {
		// TODO verify that x remains unchanged
		uint8_t addr = ((uint8_t) x) + mem[pc+1];
		data = &mem[addr];
	}
	// zero-paged indexed mode using y
	void zrpiy() {
		// TODO verify that y remains unchanged
		uint8_t addr = ((uint8_t) y) + mem[pc+1];
		data = &mem[addr];
	}
	// pre-indexed indirect mode (uses x)
	void iix() {
		//x += mem[pc+1]; TODO verify x unchanged
		uint16_t addr = (mem[((uint8_t) x) + mem[pc+1] + 1] << 8) + mem[((uint8_t) x) + mem[pc+1]];
		data = &mem[addr];
	}
	// post-indexed indirect mode (uses y)
	void iiy() {
		uint8_t imm = mem[pc+1];
		uint16_t addr = (mem[imm+1] << 8) + mem[imm] + ((uint8_t) y);
		data = &mem[addr];
	}
