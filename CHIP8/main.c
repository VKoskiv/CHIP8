//
//  main.c
//  CHIP8
//
//  Created by Valtteri Koskivuori on 29/07/16.
//  Copyright © 2016 Valtteri Koskivuori. All rights reserved.
//

#include <SDL2/SDL.h>
#include "CPU.h"
#include <time.h>

typedef unsigned char byte;

void destroyWindow(SDL_Window *window) {
	if (window != NULL) {
		SDL_DestroyWindow(window);
	}
}

void destroyRenderer(SDL_Renderer *renderer) {
	if (renderer != NULL) {
		SDL_DestroyRenderer(renderer);
	}
}

void destroyTexture(SDL_Texture *texture) {
	if (texture != NULL) {
		SDL_DestroyTexture(texture);
	}
}

void render(SDL_Renderer *renderer, SDL_Texture *texture) {
	//Clear the window
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(renderer);
	
	byte *pixels;
	int pitch = 0;
	SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);
	
	//Render the data
	byte* data = getCurrentFrame();
	memcpy(pixels, data, 64 * 32);
	
	SDL_UnlockTexture(texture);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

/*
 KEYMAPPING
 CHIP8 HEX MAP
 +-+-+-+-+
 |1|2|3|C|
 +-+-+-+-+
 |4|5|6|D|
 +-+-+-+-+
 |7|8|9|E|
 +-+-+-+-+
 |A|0|B|F|
 +-+-+-+-+
 HOST KEYBOARD
 +-+-+-+-+
 |1|2|3|4|
 +-+-+-+-+
 |Q|W|E|R|
 +-+-+-+-+
 |A|S|D|F|
 +-+-+-+-+
 |Z|X|C|V|
 +-+-+-+-+
 
 */

void setInput() {
	//Get keyboard input, then send that to the CPU
	
	SDL_PumpEvents();
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	byte input = 0xFF;
	
	if (keys[SDL_SCANCODE_1]) {
		input = 0x1;
	}
	if (keys[SDL_SCANCODE_2]) {
		input = 0x2;
	}
	if (keys[SDL_SCANCODE_3]) {
		input = 0x3;
	}
	if (keys[SDL_SCANCODE_4]) {
		input = 0xC;
	}
	if (keys[SDL_SCANCODE_Q]) {
		input = 0x4;
	}
	if (keys[SDL_SCANCODE_W]) {
		input = 0x5;
	}
	if (keys[SDL_SCANCODE_E]) {
		input = 0x6;
	}
	if (keys[SDL_SCANCODE_R]) {
		input = 0xD;
	}
	if (keys[SDL_SCANCODE_A]) {
		input = 0x7;
	}
	if (keys[SDL_SCANCODE_S]) {
		input = 0x8;
	}
	if (keys[SDL_SCANCODE_D]) {
		input = 0x9;
	}
	if (keys[SDL_SCANCODE_F]) {
		input = 0xE;
	}
	if (keys[SDL_SCANCODE_Z]) {
		input = 0xA;
	}
	if (keys[SDL_SCANCODE_X]) {
		input = 0x0;
	}
	if (keys[SDL_SCANCODE_C]) {
		input = 0xB;
	}
	if (keys[SDL_SCANCODE_V]) {
		input = 0xF;
	}

	cpu_setKeys(input);
}



int main(int argc, const char * argv[]) {
	int windowWidth = 64;
	int windowHeight = 32;
	int windowScale = 4;
	
	//Init SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL couldn't initialize, error %s",SDL_GetError());
		return false;
	}
	
	//Init window
	SDL_Window *window = SDL_CreateWindow("CHIP-8 by VKoskiv 2016", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth * windowScale, windowHeight * windowScale, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Window couldn't be created, error %s", SDL_GetError());
		return false;
	}
	
	//Init renderer
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("Renderer couldn't be created, error %s", SDL_GetError());
		return false;
	}
	
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, windowWidth, windowHeight);
	
	//Initialize the emulator
	cpu_initialize();
	cpu_loadGame("c8games/INVADERS");
	
	//Emulation loop
	for (;;) {
		cpu_emulateCycle();
		
		if (cpu_isDrawFlagSet()) {
			render(renderer, texture);
		}
		
		setInput();
		struct timespec ts;
		int milliseconds = 16;
		ts.tv_sec = milliseconds / 1000;
		ts.tv_nsec = (milliseconds % 1000) * 1000000;
		nanosleep(&ts, NULL);
	}
	
	return 4;
}
