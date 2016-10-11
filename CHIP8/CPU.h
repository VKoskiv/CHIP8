//
//  CPU.h
//  CHIP8
//
//  Created by Valtteri Koskivuori on 29/07/16.
//  Copyright © 2016 Valtteri Koskivuori. All rights reserved.
//

#ifndef CPU_h
#define CPU_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <signal.h>

//CPU debug mode prints a log of opcodes, and enables autohalt
#define CPU_DEBUG false
//If CPU debug and delayEnabled is true, use delayDebug
#define delayEnabled false

#define delayNormal 2
#define delayDebug  200

typedef unsigned char byte;

//Chip-8 memory map
// 0x000-0x1FF Chip 8 interpreter, contains font set
// 0x050-0x0A0 Used for the built in font set from 0-F
// 0x200-0xFFF Program ROM and work RAM

typedef struct
{
	unsigned short currentOP;   //2 bytes
	byte memory[4096]; //4KB
	byte V[16];		//Registers, V0 to VE, 1 byte each and VF for flags
	unsigned short I;			//Index register
	unsigned short progCounter; //Program counter, from 0x000 to 0xFFF
	
	//Graphics, a 64x32 1 bit bitmap, 2048 pixels
	byte display[64 * 32];
	//Draw flag, set to true if screen needs to be updated
	bool drawFlag;
	
	//No interrupts or hardware registers, only two timer registers that count down to 0 at 60hz
	byte delay_timer; //Used for delays
	byte sound_timer; //Used for sound, system buzzer sounds when this reaches 0
	
	//Stack
	//Some opcodes can jump to a mem location or call a subroutine
	//So we implement a stack to store the program counter before jumping
	//16 levels of stack, stack pointer denotes which stack is used
	unsigned short stack[16];
	unsigned short stackPointer;
	
	//Input
	//Chip 8 has a hex keypad with 16 keys, 0x0-0xF, this is an array to store the current state of the key
	byte key[16];
} chipCPU;

void cpu_initialize();
int cpu_load_rom(char *filepath);
void cpu_emulate_cycle();
void get_current_frame(char *buf, int count);
bool cpu_is_drawflag_set();
void cpu_set_keys(byte key);


#endif /* CPU_h */
