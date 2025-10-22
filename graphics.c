typedef struct {
	SDL_Renderer *renderer;
	SDL_Window *window;
	SDL_Texture *texture;
} Graphics;

Graphics init_graphics() {
	Graphics graphics = {};
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL failed to init!");
		exit(1);
	}
	graphics.window = SDL_CreateWindow(
		"Chip8 EMU",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		VIDEO_WIDTH * VIDEO_SCALE,
		VIDEO_HEIGHT * VIDEO_SCALE,
		0
	);
	if (!graphics.window) {
		printf("SDL failed to create window!");
	}
	graphics.renderer = SDL_CreateRenderer(
		graphics.window,
		-1,
		0
	);
	if (!graphics.renderer) {
		printf("SDL failed to create renderer!");
	}
	graphics.texture = SDL_CreateTexture(
		graphics.renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		VIDEO_WIDTH,
		VIDEO_HEIGHT
	);
	return graphics;
}

void read_input(uint8_t *keypad) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				printf("Emulation stopped.\n");
				exit(0);
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_x:
						keypad[0x0] = 1;
						break;
					case SDLK_1:
						keypad[0x1] = 1;
						break;
					case SDLK_2:
						keypad[0x2] = 1;
						break;
					case SDLK_3:
						keypad[0x3] = 1;
						break;
					case SDLK_q:
						keypad[0x4] = 1;
						break;
					case SDLK_w:
						keypad[0x5] = 1;
						break;
					case SDLK_e:
						keypad[0x6] = 1;
						break;
					case SDLK_a:
						keypad[0x7] = 1;
						break;
					case SDLK_s:
						keypad[0x8] = 1;
						break;
					case SDLK_d:
						keypad[0x9] = 1;
						break;
					case SDLK_z:
						keypad[0xA] = 1;
						break;
					case SDLK_c:
						keypad[0xB] = 1;
						break;
					case SDLK_4:
						keypad[0xC] = 1;
						break;
					case SDLK_r:
						keypad[0xD] = 1;
						break;
					case SDLK_f:
						keypad[0xE] = 1;
						break;
					case SDLK_v:
						keypad[0xF] = 1;
						break;
					default:
						break;
				}
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
					case SDLK_x:
						keypad[0x0] = 0;
						break;
					case SDLK_1:
						keypad[0x1] = 0;
						break;
					case SDLK_2:
						keypad[0x2] = 0;
						break;
					case SDLK_3:
						keypad[0x3] = 0;
						break;
					case SDLK_q:
						keypad[0x4] = 0;
						break;
					case SDLK_w:
						keypad[0x5] = 0;
						break;
					case SDLK_e:
						keypad[0x6] = 0;
						break;
					case SDLK_a:
						keypad[0x7] = 0;
						break;
					case SDLK_s:
						keypad[0x8] = 0;
						break;
					case SDLK_d:
						keypad[0x9] = 0;
						break;
					case SDLK_z:
						keypad[0xA] = 0;
						break;
					case SDLK_c:
						keypad[0xB] = 0;
						break;
					case SDLK_4:
						keypad[0xC] = 0;
						break;
					case SDLK_r:
						keypad[0xD] = 0;
						break;
					case SDLK_f:
						keypad[0xE] = 0;
						break;
					case SDLK_v:
						keypad[0xF] = 0;
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
}

void render(Graphics *graphics, uint32_t *buffer) {
	SDL_UpdateTexture(graphics->texture, NULL, buffer, VIDEO_PITCH);
	SDL_RenderClear(graphics->renderer);
	SDL_RenderCopy(graphics->renderer, graphics->texture, NULL, NULL);
	SDL_RenderPresent(graphics->renderer);
}

