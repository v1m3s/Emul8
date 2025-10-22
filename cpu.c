// AND [mask] -> clearing bits / checking a bit
// i.e 0xF6 && 0x01 = 0x00 (checking rightmost bit)
// Use this in switch statements
// OR [mask] -> setting bits
// NOT -> toggling bits
// XOR [mask] -> detection

// Chip 8 architecture
typedef struct {
	uint8_t memory[4096];
	uint8_t registers[16];
	uint16_t index;
	uint16_t pc;
	uint16_t stack[16];
	uint8_t stack_ptr;
	uint8_t delay_timer;
	uint8_t sound_timer;
	uint8_t keypad[16];
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT];  // Each pixel is represented by 32 bits
	uint16_t opcode;
} Chip8;

// Random byte generation (seeded in main)
uint8_t random_byte() {
	uint8_t r = rand() % 256;  // Not ideal
	return r;
}

// Instruction set functions

// Run native machine code (unused)
void op_0nnn(Chip8 *cpu) {
	// Not implemented
}

// Clear the screen
void op_00E0(Chip8 *cpu) {
	memset(cpu->video, 0, sizeof(cpu->video));
}

// Return from a subroutine
void op_00EE(Chip8 *cpu) {
	--cpu->stack_ptr;
	cpu->pc = cpu->stack[cpu->stack_ptr];
}

// Jump to address
void op_1nnn(Chip8 *cpu) {
	cpu->pc = cpu->opcode & 0x0FFFu;
}

// Jump to subroutine
void op_2nnn(Chip8 *cpu) {
	cpu->stack[cpu->stack_ptr] = cpu->pc;
	++cpu->stack_ptr;
	cpu->pc = cpu->opcode & 0x0FFFu;
}

// Skip next instruction if Vx == nn
void op_3xnn(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t byte = cpu->opcode & 0x00FFu;
	if (cpu->registers[Vx] == byte) {
		cpu->pc += 2;
	}
}

// Skip next instruction if Vx != nn
void op_4xnn(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t byte = cpu->opcode & 0x00FFu;
	if (cpu->registers[Vx] != byte) {
		cpu->pc += 2;
	}
}

// Skip next instruction if Vx == Vy
void op_5xy0(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	if (cpu->registers[Vx] == cpu->registers[Vy]) {
		cpu->pc += 2;
	}
}

// Store value nn in register Vx
void op_6xnn(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t byte = cpu->opcode & 0x00FFu;
	cpu->registers[Vx] = byte;
}

// Add value nn to register Vx
void op_7xnn(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t byte = cpu->opcode & 0x00FFu;
	cpu->registers[Vx] = (cpu->registers[Vx] + byte) & 0xFFu;
}

// Store the value of register Vy in Vx
void op_8xy0(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	cpu->registers[Vx] = cpu->registers[Vy];
}

// Set register Vx to the value of Vx OR Vy
void op_8xy1(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	cpu->registers[Vx] = cpu->registers[Vx] | cpu->registers[Vy];
}

// Set register Vx to the value of Vx AND Vy
void op_8xy2(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	cpu->registers[Vx] = cpu->registers[Vx] & cpu->registers[Vy];
}

// Set register Vx to the value of Vx XOR Vy
void op_8xy3(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	cpu->registers[Vx] = cpu->registers[Vx] ^ cpu->registers[Vy];
}

// Set register Vx to the value of Vx + Vy
void op_8xy4(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	uint16_t result = cpu->registers[Vx] + cpu->registers[Vy];
	if (result > 255u) {
		cpu->registers[0xFu] = 1;  // Set Vf to 1 if a carry occurs
	}
	else {
		cpu->registers[0xFu] = 0;  // Set Vf to 0 if it does not
	}
	cpu->registers[Vx] = result & 0xFFu;
}

// Set register Vx to the value of Vx - Vy
void op_8xy5(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	if (cpu->registers[Vy] > cpu->registers[Vx]) {
		cpu->registers[0xFu] = 0;  // Set Vf to 0 if a borrow occurs
	}
	else {
		cpu->registers[0xFu] = 1;  // Set Vf to 1 if it doesn't
	}
	cpu->registers[Vx] = cpu->registers[Vx] - cpu->registers[Vy];
}

// Set register Vx to the value of Vy right-shifted by 2
// NOTE: in some implementations, Vx is right-shifted by
// one (Vy is ignored, Vf is set in the same way)
void op_8xy6(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	cpu->registers[0xFu] = cpu->registers[Vy] & 0x1u;
	cpu->registers[Vx] = cpu->registers[Vy] >> 1u;
}

// Wrong implementation: use if ROMs break with the correct instruction
void op_8xy6_wrong(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->registers[0xFu] = cpu->registers[Vx] & 0x1u;
	cpu->registers[Vx] = cpu->registers[Vx] >> 1u;
}

// Set register Vx to the value of Vy - Vx
void op_8xy7(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	if (cpu->registers[Vx] > cpu->registers[Vy]) {
		cpu->registers[0xFu] = 0;  // Set Vf to 0 if a borrow occurs
	}
	else {
		cpu->registers[0xFu] = 1;  // Set Vf to 1 if it doesn't
	}
	cpu->registers[Vx] = cpu->registers[Vy] - cpu->registers[Vx];
}

// Set register Vx to the value of Vy left-shifted
// NOTE: in some implementations, Vx is left-shifted
// (Vy is ignored, Vf is set in the same way)
void op_8xyE(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	cpu->registers[0xFu] = cpu->registers[Vy] & 0x80u;
	cpu->registers[Vx] = cpu->registers[Vy] << 1u;
}

// Wrong implementation: use if ROMs break with the correct instruction
void op_8xyE_wrong(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->registers[0xFu] = cpu->registers[Vx] & 0x80u;
	cpu->registers[Vx] = cpu->registers[Vx] << 1u;
}

// Skip next instruction if Vx != Vy
void op_9xy0(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	if (cpu->registers[Vx] != cpu->registers[Vy]) {
		cpu->pc += 2;
	}
}

// Store memory address nnn in the index register
void op_Annn(Chip8 *cpu) {
	uint16_t address = cpu->opcode & 0x0FFFu;
	cpu->index = address;
}

// Jump to address nnn + V0
void op_Bnnn(Chip8 *cpu) {
	uint16_t address = cpu->opcode & 0x0FFFu;
	cpu->pc = address + cpu->registers[0x0u];
}

// Set Vx to a random byte with a mask of nn
void op_Cxnn(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t mask = cpu->opcode & 0x00FF;
	cpu->registers[Vx] = random_byte() & mask;
}

// Draw a sprite at Vx, Vy with n bytes of sprite data
// starting at the address stored in the index register
// Set Vf to 1 if any set pixels are changed to unset,
// and 0 otherwise
void op_Dxyn(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (cpu->opcode & 0x00F0u) >> 4u;
	uint8_t height = cpu->opcode & 0x000Fu;  // Number of bytes to draw to the screen (sprite height)
	cpu->registers[0xFu] = 0;  // Set Vf to 0 if no set bytes are unset
	uint8_t row_pos = cpu->registers[Vy] % VIDEO_HEIGHT;  // Screen coordinates to draw to (top left)
	uint8_t col_pos = cpu->registers[Vx] % VIDEO_WIDTH;
	for (uint8_t row = 0; row < height; ++row) {
		for (uint8_t col = 0; col < 8; ++col) {
			uint8_t sprite_pixel = cpu->memory[cpu->index + row] & (0x80u >> col);
			uint32_t *screen_pixel = &cpu->video[(row_pos + row) * VIDEO_WIDTH + (col_pos + col)];
			if (sprite_pixel) {
				if (*screen_pixel == 0xFFFFFFFFu) {
					cpu->registers[0xFu] = 1;  // Set Vf to 1 if any set bytes are unset
				}
				*screen_pixel ^= 0xFFFFFFFFu;
			}
		}
	}
}

// Skip the next instruction if the key equal to the value of register Vx is pressed
void op_Ex9E(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t key = cpu->registers[Vx];
	if (cpu->keypad[key]) {
		cpu->pc += 2;
	}
}

// Skip the next instruction if the key equal to the value of register Vx is not pressed
void op_ExA1(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t key = cpu->registers[Vx];
	if (!cpu->keypad[key]) {
		cpu->pc += 2;
	}
}

// Store the current value of the delay timer in Vx
void op_Fx07(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->registers[Vx] = cpu->delay_timer;
}

// Wait for a keypress, and record the key in register Vx
void op_Fx0A(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	bool flag = false;
	for (uint8_t i = 0; i < 16; ++i) {
		if (cpu->keypad[i]) {
			cpu->registers[Vx] = i;
			flag = true;
			break;
		}
	}
	if (!flag) {
		cpu->pc -= 2;
	}
}

// Set the delay timer to the value of register Vx
void op_Fx15(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->delay_timer = cpu->registers[Vx];
}

// Set the sound timer to the value of register Vx
void op_Fx18(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->sound_timer = cpu->registers[Vx];
}

// Add the value of register Vx to the value of the index register
void op_Fx1E(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->index += cpu->registers[Vx];
}

// Set the index register to the memory address of the sprite data
// of the hexadecimal digit stored in register Vx
void op_Fx29(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	cpu->index = FONTSET_ADDRESS + (5 * cpu->registers[Vx]);
}

// Store the binary-coded decimal equivilant to the value of 
// register Vx at the three consecutive addresses starting at the
// address stored in the index register
void op_Fx33(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	uint8_t value = cpu->registers[Vx];
	for (uint8_t i = 0; i < 3; ++i) {  // Stores ones, tens, hundreds digits
		cpu->memory[cpu->index + (2u - i)] = value % 10;
		value /= 10;
	}
}

// Stores the values of registers V0-Vx inclusive in memory starting
// at the address stored in the index register
// The value of x + 1 is added to the index register
// NOTE: in some implementations, the value of the index register is unchanged
void op_Fx55(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i) {
		cpu->memory[cpu->index + i] = cpu->registers[i];
	}
	cpu->index += Vx + 1u;
}

// Wrong implementation: use if ROMs break with the correct instruction
void op_Fx55_wrong(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i) {
		cpu->memory[cpu->index + i] = cpu->registers[i];
	}
}


// Loads the values of registers V0-Vx inclusive from memory starting
// at the address stored in the index register
// The value of x + 1 is added to the index register
// NOTE: in some implementations, the value of the index register is unchanged
void op_Fx65(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i) {
		cpu->registers[i] = cpu->memory[cpu->index + i];
	}
	cpu->index += Vx + 1u;
}

// Wrong implementation: use if ROMs break with the correct instruction
void op_Fx65_wrong(Chip8 *cpu) {
	uint8_t Vx = (cpu->opcode & 0x0F00u) >> 8u;
	for (uint8_t i = 0; i <= Vx; ++i) {
		cpu->registers[i] = cpu->memory[cpu->index + i];
	}
}

// Placeholder function
void op_NULL(Chip8 *cpu) {}
// General opcode function type
typedef void op_func(Chip8 *);
// Function pointer table
op_func *jump_table[0xFu + 1];
// Sub-table 0
op_func *jt_0[0xEu + 1];
void call_jt_0(Chip8 *cpu) {
	jt_0[cpu->opcode & 0x000Fu](cpu);
}
// Sub-table 8
op_func *jt_8[0xEu + 1];
void call_jt_8(Chip8 *cpu) {
	jt_8[cpu->opcode & 0x000Fu](cpu);
}
// Sub-table E
op_func *jt_E[0xEu + 1];
void call_jt_E(Chip8 *cpu) {
	jt_E[cpu->opcode & 0x000Fu](cpu);
}
//Sub-table F
op_func *jt_F[0x65u + 1];
void call_jt_F(Chip8 *cpu) {
	jt_F[cpu->opcode & 0x00FFu](cpu);
}
// Initialise tables
void init_tables() {
	// Main table
	jump_table[0x0u] = call_jt_0;
	jump_table[0x1u] = op_1nnn;
	jump_table[0x2u] = op_2nnn;
	jump_table[0x3u] = op_3xnn;
	jump_table[0x4u] = op_4xnn;
	jump_table[0x5u] = op_5xy0;
	jump_table[0x6u] = op_6xnn;
	jump_table[0x7u] = op_7xnn;
	jump_table[0x8u] = call_jt_8;
	jump_table[0x9u] = op_9xy0;
	jump_table[0xAu] = op_Annn;
	jump_table[0xBu] = op_Bnnn;
	jump_table[0xCu] = op_Cxnn;
	jump_table[0xDu] = op_Dxyn;
	jump_table[0xEu] = call_jt_E;
	jump_table[0xFu] = call_jt_F;
	// Sub-table 0
	jt_0[0x0u] = op_00E0;
	jt_0[0xEu] = op_00EE;
	// Sub-table 8
	jt_8[0x0u] = op_8xy0;
	jt_8[0x1u] = op_8xy1;
	jt_8[0x2u] = op_8xy2;
	jt_8[0x3u] = op_8xy3;
	jt_8[0x4u] = op_8xy4;
	jt_8[0x5u] = op_8xy5;
	jt_8[0x6u] = op_8xy6;
	jt_8[0x7u] = op_8xy7;
	jt_8[0xEu] = op_8xyE;
	// Sub-table E
	jt_E[0x1u] = op_ExA1;
	jt_E[0xEu] = op_Ex9E;
	// Sub-table F
	jt_F[0x07u] = op_Fx07;
	jt_F[0x0Au] = op_Fx0A;
	jt_F[0x15u] = op_Fx15;
	jt_F[0x18u] = op_Fx18;
	jt_F[0x1Eu] = op_Fx1E;
	jt_F[0x29u] = op_Fx29;
	jt_F[0x33u] = op_Fx33;
	jt_F[0x55u] = op_Fx55;
	jt_F[0x65u] = op_Fx65;
}

// Initialise
Chip8 init_cpu() {
	Chip8 cpu = {
		.memory = {0},
		.registers = {0},
		.index = 0,
		.pc = PROGRAM_ADDRESS,
		.stack = {0},
		.stack_ptr = 0,
		.delay_timer = 0,
		.sound_timer = 0,
		.keypad = {0},
		.video = {0},
	};
	// Load fontset
	for (uint8_t i = 0; i < 80; ++i) {
		cpu.memory[FONTSET_ADDRESS + i] = FONTSET[i];
	}
	init_tables();
	// Return
	return cpu;
}

// One FDE cycle
void cycle(Chip8 *cpu) {
	cpu->opcode = (cpu->memory[cpu->pc] << 8u) | cpu->memory[cpu->pc + 1u];
	cpu->pc += 2u;
	jump_table[(cpu->opcode & 0xF000u) >> 12u](cpu);
	if (cpu->delay_timer > 0) {
		--cpu->delay_timer;
	}
	if (cpu->sound_timer > 0) {
		--cpu->sound_timer;
	}
}

