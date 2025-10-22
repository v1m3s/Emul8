void load_rom(Chip8 *cpu, char *path, bool display_code) {
	int byte;
	FILE *rom = fopen(path, "rb");
	uint16_t i = 0x0u;
	// For displaying program code
	if (display_code) {
		printf("Program code:\n");
	}
	// Read rom into memory
	while ((byte = fgetc(rom)) != EOF) {
		if (i <= 0xFFFu) {
			// For displaying program code
			if (display_code) {
				printf("\t%x", byte);
				if (!((i + 1) % 8)) {
					printf("\n");
				}
			}
			// Load program byte into memory
			cpu->memory[PROGRAM_ADDRESS + i] = (uint8_t)byte;
			++i;
		}
		else {
			printf("Program is too large to load!");
			break;
		}
	}
	fclose(rom);
}
