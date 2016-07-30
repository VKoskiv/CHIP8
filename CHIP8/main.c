//
//  main.c
//  CHIP8
//
//  Created by Valtteri Koskivuori on 29/07/16.
//  Copyright © 2016 Valtteri Koskivuori. All rights reserved.
//

/*
 TODO:
 Add input
 Add output
 Add sleep() to run the emulation loop at 60hz
 */

#include <SDL2/SDL.h>
#include "CPU.h"

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
		cpu_setKeys();
		
		
	}
	
	return 4;
}
