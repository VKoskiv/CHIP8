//
//  main.c
//  CHIP8
//
//  Created by Valtteri Koskivuori on 29/07/16.
//  Copyright © 2016-2020 Valtteri Koskivuori. All rights reserved.
//

#include <SDL2/SDL.h>
#include "CPU.h"
#include <time.h>

// THREADING
#ifdef WINDOWS
#include <Windows.h>
#else
#include <pthread.h>
#endif

#include <stdbool.h>
#include <stdint.h>

struct thread {
#ifdef WINDOWS
	HANDLE thread_handle;
	DWORD thread_id;
#else
	pthread_t thread_id;
#endif
	int thread_num;
	bool threadComplete;
	void *(*threadFunc)(void *);
};

// Multiplatform thread stub
#ifdef WINDOWS
DWORD WINAPI threadStub(LPVOID arg) {
#else
	void *threadStub(void *arg) {
#endif
		return ((struct thread*)arg)->threadFunc(arg);
	}
	
	void checkThread(struct thread *t) {
#ifdef WINDOWS
		WaitForSingleObjectEx(t->thread_handle, INFINITE, FALSE);
#else
		if (pthread_join(t->thread_id, NULL)) {
			printf("Thread %i frozen.", t->thread_num);
		}
#endif
	}
	
	int startThread(struct thread *t) {
#ifdef WINDOWS
		t->thread_handle = CreateThread(NULL, 0, threadStub, t, 0, &t->thread_id);
		if (t->thread_handle == NULL) return -1;
		return 0;
#else
		pthread_attr_t attribs;
		pthread_attr_init(&attribs);
		pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_JOINABLE);
		int ret = pthread_create(&t->thread_id, &attribs, threadStub, t);
		pthread_attr_destroy(&attribs);
		return ret;
#endif
}

bool emulatorRunning = true;

void (*signal(int signo, void (*func )(int)))(int);
typedef void sigfunc(int);
sigfunc *signal(int, sigfunc*);

void sig_handler(int sig) {
	if (sig == SIGINT) {
		emulatorRunning = false;
	}
}

void destroy_window(SDL_Window *window) {
	if (window != NULL) {
		SDL_DestroyWindow(window);
	}
}

void destroy_renderer(SDL_Renderer *renderer) {
	if (renderer != NULL) {
		SDL_DestroyRenderer(renderer);
	}
}

void destroy_texture(SDL_Texture *texture) {
	if (texture != NULL) {
		SDL_DestroyTexture(texture);
	}
}

void render(SDL_Renderer *renderer) {
	//Clear the window
	SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0x0);
	SDL_RenderClear(renderer);
	
	//Render the data
	char data[2048] = {0};
	get_current_frame(data, 2048);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	for (int y = 0; y < 32; ++y) {
		for (int x = 0; x < 64; ++x) {
			if (data[(y*64) + x] == 0) {
				//Don't draw ¯\_(ツ)_/¯
			} else {
				SDL_RenderDrawPoint(renderer, x, y);
			}
		}
	}
	SDL_RenderPresent(renderer);
}

void sleepMSec(int ms) {
#ifdef WINDOWS
	Sleep(ms);
#elif __APPLE__
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
#elif __linux__
	usleep(ms * 1000);
#endif
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

void set_input() {
	//Get keyboard input, then send that to the CPU
	
	SDL_PumpEvents();
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	byte input[16] = {0x0};
	
	//Prevent keys from sticking when pressing more than one at a time
	cpu_set_keys(input);
	
	if (keys[SDL_SCANCODE_1]) {
		input[0x1] = 0x1;
	}
	if (keys[SDL_SCANCODE_2]) {
		input[0x2] = 0x1;
	}
	if (keys[SDL_SCANCODE_3]) {
		input[0x3] = 0x1;
	}
	if (keys[SDL_SCANCODE_4]) {
		input[0xC] = 0x1;
	}
	if (keys[SDL_SCANCODE_Q]) {
		input[0x4] = 0x1;
	}
	if (keys[SDL_SCANCODE_W]) {
		input[0x5] = 0x1;
	}
	if (keys[SDL_SCANCODE_E]) {
		input[0x6] = 0x1;
	}
	if (keys[SDL_SCANCODE_R]) {
		input[0xD] = 0x1;
	}
	if (keys[SDL_SCANCODE_A]) {
		input[0x7] = 0x1;
	}
	if (keys[SDL_SCANCODE_S]) {
		input[0x8] = 0x1;
	}
	if (keys[SDL_SCANCODE_D]) {
		input[0x9] = 0x1;
	}
	if (keys[SDL_SCANCODE_F]) {
		input[0xE] = 0x1;
	}
	if (keys[SDL_SCANCODE_Z]) {
		input[0xA] = 0x1;
	}
	if (keys[SDL_SCANCODE_X]) {
		input[0x0] = 0x1;
	}
	if (keys[SDL_SCANCODE_C]) {
		input[0xB] = 0x1;
	}
	if (keys[SDL_SCANCODE_V]) {
		input[0xF] = 0x1;
	}
	
	cpu_set_keys(input);
}

void *decrement_counters(void *none) {
	while (emulatorRunning) {
		cpu_decrement_counters();
		sleepMSec(16);
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	int windowWidth = 64;
	int windowHeight = 32;
	int windowScale = 16; //How big the pixels are
	
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	
	
	//Init SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL couldn't initialize, error %s",SDL_GetError());
		return false;
	}
	
	SDL_WindowFlags flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
	
	//Init window
	window = SDL_CreateWindow("CHIP-8 by VKoskiv 2016-2020",
							  SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED,
							  windowWidth * (windowScale/2),
							  windowHeight * (windowScale/2), flags);
	if (window == NULL) {
		printf("Window couldn't be created, error %s", SDL_GetError());
		return false;
	}
	
	//Init renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("Renderer couldn't be created, error %s", SDL_GetError());
		return false;
	}
	SDL_RenderSetScale(renderer, windowScale, windowScale);
	
	//Initialize the emulator
	cpu_initialize();
	
	if (argc == 2) {
		switch (cpu_load_rom(argv[1])) {
			case -1:
				printf("Couldn't find the ROM file! (Check working dir/path)\n");
				return -1;
				break;
			case -2:
				printf("ROM too big\n");
				return -1;
				break;
				
			default:
				printf(" bytes loaded.\n");
				break;
		}
	} else {
		printf("Please provide a ROM filepath as argument!\n");
		return -1;
	}
	
	//Start counter decrement loop.
	
	struct thread decrementer = (struct thread){.threadFunc = decrement_counters};
	
	startThread(&decrementer);
	
	//Emulation loop
	do {
		//Check for CTRL-C
		if (signal(SIGINT, sig_handler) == SIG_ERR) {
			printf("Couldn't catch SIGINT\n");
		}
		//Run the CPU cycle
		cpu_emulate_cycle();
		//Draw if needed
		if (cpu_is_drawflag_set()) {
			render(renderer);
		}
		//Check if CPU has halted
		if (cpu_has_halted()) {
			emulatorRunning = false;
		}
		//Set keyboard input to CPU
		set_input();
		//Delay
		
		int ms;
		
		if (CPU_DEBUG && delayEnabled) {
			ms = delayDebug;
		}
		else {
			ms = delayNormal;
		}
		sleepMSec(ms);
	} while (emulatorRunning);
	
	destroy_renderer(renderer);
	destroy_window(window);
	
	return 0;
}

