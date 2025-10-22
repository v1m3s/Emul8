// Standard libraries
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// SDL
#include "SDL.h"
#include "SDL_timer.h"

// Consts
#define VIDEO_WIDTH 64
#define VIDEO_HEIGHT 32
#define VIDEO_SCALE 10
#define VIDEO_PITCH 256  // bytes per row
const static uint16_t PROGRAM_ADDRESS = 0x200u;
const static uint8_t FONTSET[80] = { 
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
const uint16_t FONTSET_ADDRESS = 0x050u;

// Program files
#include "cpu.c"
#include "romhandler.c"
#include "graphics.c"

// Main
int main(int argc, char **argv) { 
	// Declaring this here so it can be changed by delay arg
	uint64_t delay = 2;  // Apparently accurate to the original hardware
	char *delay_arg;  // For reading command line arg
	// Determining which rom to load
	char path[50];  // rom name can be up to 40 characters
	if ((argc == 1) || (argc > 3)) {
		printf("Usage: emul8 <rom name (required)> <delay (optional)>\n");
		exit(1);
	}
	else {
		char *program_name = argv[1];
		snprintf(path, strlen(program_name) + 11, "Games/%s.ch8", program_name);  // strlen("Games/.ch8") = 11
		printf("Loading ROM: %s\n", path);
		// Setting delay (if supplied)
		if (argc == 3) {
			delay_arg = argv[2];
			delay = atoi(delay_arg);
		}
	}
	// Init everything
	srand(time(NULL));  // Seed for rng
	Chip8 cpu = init_cpu();
	load_rom(&cpu, path, false);  // last option is to display program code
	Graphics graphics = init_graphics();
	// Main loop
	printf("Running...\n");
	while (true) {
		uint64_t wait = SDL_GetTicks64() + delay;
		while(SDL_GetTicks64() < wait) {};
		read_input(cpu.keypad);
		cycle(&cpu);
		render(&graphics, cpu.video);
	}
	return 0;
}

