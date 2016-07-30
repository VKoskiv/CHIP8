//
//  CPU.c
//  CHIP8
//
//  Created by Valtteri Koskivuori on 29/07/16.
//  Copyright © 2016 Valtteri Koskivuori. All rights reserved.
//

#include "CPU.h"

//The Chip-8 font set includes numvers from 0 to 9, and ABCDEF
//Only the first four bits are used for drawing a number or character
//So the first four bits from each of these columns is drawn under each other, to form the character
unsigned char mainFontset[80] = {
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

chipCPU mainCPU;

void cpu_initialize() {
	//Init registers and memory once
	//The program counter starts at 0x200, and that's where we'll load the program code
	mainCPU.progCounter = 0x200;
	mainCPU.currentOP = 0;		//Reset the current opcode
	mainCPU.I = 0;				//Reset the index register
	mainCPU.stackPointer = 0;	//Reset the stack pointer
	mainCPU.drawFlag = false;
	
	//Clear the display
	for (int i = 0; i <= 2048; i++) {
		mainCPU.display[i] = 0;
	}
	//Clear the stack
	for (int i = 0; i <= 16; i++) {
		mainCPU.stack[i] = 0;
	}
	//Clear the registers V0-VF
	for (int i = 0; i <= 16; i++) {
		mainCPU.V[i] = 0;
	}
	//Clear the memory
	for (int i = 0; i <= 4096; i++) {
		mainCPU.memory[i] = 0;
	}
	
	//Load the fontset
	for (int i = 0; i <= 80; i++) {
		mainCPU.memory[i] = mainFontset[i];
	}
	
	//Reset timers
	mainCPU.delay_timer = 0;
	mainCPU.sound_timer = 0;
}

byte* getCurrentFrame() {
	return mainCPU.display;
}

int cpu_loadGame(char *filepath) {
	
	FILE *inputFile = fopen(filepath, "r");
	if (!inputFile) {
		return -1;
	}
	fseek(inputFile, 0L, SEEK_END);
	long size = ftell(inputFile);
	unsigned char buffer[size];
	rewind(inputFile);
	
	for (int i = 0; i <= size; i++) {
		buffer[i] = getc(inputFile);
	}
	
	
	for (int i = 0; i <= sizeof(buffer); i++) {
		mainCPU.memory[i + 512] = buffer[i]; //0x200 == 512, which is where the CPU starts execution
	}
	return 0;
}

void cpu_emulateCycle() {
	//Fetch opcode
	//Each opcode is two bytes, so we shift left by 8 to add zeros after the first byte
	//Then AND the second byte to add it after the first byte
	mainCPU.currentOP = mainCPU.memory[mainCPU.progCounter] << 8 | mainCPU.memory[mainCPU.progCounter + 1];
	
	//Decode opcode and execute
	switch (mainCPU.currentOP & 0xF000) { //Compare the FIRST 4 bits
		case 0x0000:
			//In some cases the first 4 bits don't tell us the opcode, in that case check the last 4 bits
			switch (mainCPU.currentOP & 0x000F) { //Compare the LAST 4 bits
				case 0x0000: //Must be 0x00E0, which clears the screen
					//Execute
					break;
				case 0x000E: //Must be 0x00EE, which returns from subroutine
					//Execute
					break;
					
				default:
					printf("Unknown opcode [0x0000]: 0x%X\n", mainCPU.currentOP);
					break;
			}
			break;
			
		case 0x1000: // 0x1NNN: Jump to address NNN
			//Don't increment the program counter because we're jumping to an address
			mainCPU.progCounter = mainCPU.currentOP & 0x0FFF;
			break;
			
		case 0x2000: // 0x2NNN: Call subroutine at NNN
			//Since we're calling a subroutine at a specific address, we don't increase program counter by 2
			mainCPU.stack[mainCPU.stackPointer] = mainCPU.progCounter;
			++mainCPU.stackPointer;
			mainCPU.progCounter = mainCPU.currentOP & 0x0FFF;
			break;
			
		case 0x3000: // 0x3XNN: Skip the next instruction if VX equals NN
			if (mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] == (mainCPU.currentOP & 0x00FF))
				mainCPU.progCounter += 4;
			else
				mainCPU.progCounter += 2;
			break;
			
		case 0x4000: // 0x4XNN: Skip the next instruction if VX doesn't equal NN
			if (mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] != (mainCPU.currentOP & 0x00FF))
				mainCPU.progCounter += 4;
			else
				mainCPU.progCounter += 2;
			break;
			
		case 0x5000: // 0x5XY0: Skip the next instruction if VX equals VY
			if (mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] == mainCPU.V[(mainCPU.currentOP & 0x00F0) >> 4])
				mainCPU.progCounter += 4;
			else
				mainCPU.progCounter += 2;
			break;
			
		case 0x6000: // 0x6XNN: Set VX to NN
			mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] = (mainCPU.currentOP & 0x00FF);
			mainCPU.progCounter += 2;
			break;
			
		case 0x7000: // 0x7XNN: Add NN to VX
			//Check for overflow
			//FIXME: Spec does not tell to set carry flag here, so disable this if it freaks out, probably maybe shouldn't cause trouble?
			if ((mainCPU.currentOP & 0x00FF) > (0xFF - mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8]))
				mainCPU.V[0xF] = 1; //Carry
			else
				mainCPU.V[0xF] = 0; //No overflow, no carry
			mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] += (mainCPU.currentOP & 0x00FF);
			mainCPU.progCounter += 2;
			break;
			
		case 0x8000: //0x8000 has 9 different opcodes, so we check the last 4 bits again to see which one it is
			switch (mainCPU.currentOP & 0x000F) {
				case 0x0000: // 0x8XY0: Set VX to the value of VY
					mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] = mainCPU.V[(mainCPU.currentOP & 0x00F0) >> 4];
					mainCPU.progCounter += 2;
					break;
				case 0x0001: // 0x8XY1: Set VX to VX or VY
					mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] = (mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] | mainCPU.V[(mainCPU.currentOP & 0x00F0) >> 4]);
					mainCPU.progCounter += 2;
					break;
				case 0x0002: // 0x8XY2: Set VX to VX and VY
					mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] = (mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] & mainCPU.V[(mainCPU.currentOP & 0x00F0) >> 4]);
					mainCPU.progCounter += 2;
					break;
				case 0x0003: // 0x8XY3: Set VX to VX xor VY
					mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] = (mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] ^ mainCPU.V[(mainCPU.currentOP & 0x00F0) >> 4]);
					mainCPU.progCounter += 2;
					break;
				case 0x0004: // 0x8XY4: Add VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
					//Remember to set VF to carry if overflows
					if (mainCPU.V[(mainCPU.currentOP & 0x00F0) >> 4] > (0xFF - mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8]))
						mainCPU.V[0xF] = 1; //carry
					else
						mainCPU.V[0xF] = 0;
					mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] += mainCPU.V[(mainCPU.currentOP & 0x00F0) >> 4];
					mainCPU.progCounter += 2;
					break;
				case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					break;
				case 0x0006: // 0x8XY6: Shift VX right by one. VF is set to the value of the least significant bit of VX before the shift
					break;
				case 0x0007: // 0x8XY7: Set VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
					break;
				case 0x000E: // 0x8XYE: Shift VX left by one. VF is set to the value of the most significant bit of VX before the shift
					break;
					
				default:
					printf("Unknown opcode [0x8000]: 0x%X", mainCPU.currentOP);
					break;
			}
			break;
		
		case 0x9000: // 0x9XY0: Skip the next instruction if VX doesn't equal VY
			if (mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] != mainCPU.V[(mainCPU.currentOP & 0x00F0) >> 4])
				mainCPU.progCounter += 4;
			else
				mainCPU.progCounter += 2;
			break;
			
		case 0xA000: // 0xANNN: Set I to the address NNN
			mainCPU.I = mainCPU.currentOP & 0x0FFF;
			mainCPU.progCounter += 2; //Every instruction is 2 bytes, so increment PC by 2
			break;
			
		case 0xB000: // 0xBNNN: Jump to the address NNN plus V0
			break;
		
		case 0xC000: // 0xCXNN: Set VX to the result of a bitwise and operation on a random number and NN
			break;
			
		case 0xD000: // 0xDXYN: Draw a sprite at coordinate (VX,VY) that has a width of 8px and a height of Npx. Each row of 8 pixels is read as bit-coded starting from mem location I; I value doesn't change after the execution of this instruction. As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn't happen
			{
				unsigned short x = mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8];
				unsigned short y = mainCPU.V[(mainCPU.currentOP & 0x00F0) >> 4];
				unsigned short height = mainCPU.currentOP & 0x000F;
				unsigned short pixel;
				
				mainCPU.V[0xF] = 0; //No collision by default
				for (int yline = 0; yline < height; yline++) {
					pixel = mainCPU.memory[mainCPU.I + yline];
					for (int xline = 0; xline < 8; xline++) {
						if ((pixel & (0x80 >> xline)) != 0) {
							if (mainCPU.display[(x + xline + ((y + yline) * 64))] == 1) {
								mainCPU.V[0xF] = 1; //Collision happened
								mainCPU.display[x + xline + ((y + yline) * 64)] ^= 1;
							}
						}
					}
				}
				mainCPU.drawFlag = true; //We've altered the display array, therefore set drawflag to true to update the screen
				mainCPU.progCounter += 2;
			}
			
			break;
			
		case 0xE000: //Input opcodes
			switch (mainCPU.currentOP & 0x00FF) {
				case 0x009E: // 0xEX9E: Skip the next instruction if the key stored in VX is pressed
					if (mainCPU.key[(mainCPU.currentOP & 0x0F00) >> 8] != 0)
						mainCPU.progCounter += 4;
					else
						mainCPU.progCounter += 2;
					break;
				case 0x00A1: // 0xEXA1: Skip the next instruction if the key stored in VX isn't pressed
					if (mainCPU.key[(mainCPU.currentOP & 0x0F00) >> 8] == 0)
						mainCPU.progCounter += 4;
					else
						mainCPU.progCounter += 2;
					break;
					
				default:
					printf("Unknown opcode [0xE000]: 0x%X", mainCPU.currentOP);
					break;
			}
			break;
		
		case 0xF000:
			switch (mainCPU.currentOP & 0x00FF) {
				case 0x0007: // 0xFX07: Set VX to the value of the delay timer
					break;
				case 0x000A: // 0xFX0A: Wait for key press, then store in VX
					break;
				case 0x0015: // 0xFX15: Set the delay timer to VX
					break;
				case 0x0018: // 0xFX18: Set the sound timer to VX
					break;
				case 0x001E: // 0xFX1E: Add VX to I
					break;
				case 0x0029: // 0xFX29: Set I to the location of the sprite for the character in VX. Characters 0-F in hex are represented by a 4x5 font (mainFontset)
					break;
				case 0x0033: // 0xFX33: Store the binary-coded decimal representation of VX, with the most significant of three digits at the address I, in the middle digit at I+1, and the least significant digit at I+2. So basically take the decimal representation of VX, place the hundreds digit in memory at I, tens digit at I+1, and ones at I+2
					mainCPU.memory[mainCPU.I]	  =  mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] / 100;
					mainCPU.memory[mainCPU.I + 1] = (mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] / 10) % 10;
					mainCPU.memory[mainCPU.I + 2] = (mainCPU.V[(mainCPU.currentOP & 0x0F00) >> 8] % 100) % 10;
					break;
				case 0x0055: // 0xFX55: Stores V0 to VX (Including VX) in memory starting at address I
					break;
				case 0x0065: // 0xFX65: Fills V0 to VX (Including VX) with values from memory starting at address I
					break;
					
				default:
					break;
			}
			break;
			
		default:
			printf("ERROR: Unknown opcode: 0x%X\n", mainCPU.currentOP);
			break;
	}
	
	//Update timers
	if (mainCPU.delay_timer != 0) {
		mainCPU.delay_timer--;
	}
	if (mainCPU.sound_timer != 0) {
		if (mainCPU.sound_timer == 1)
			printf("BEEP!"); //TODO: Make this beep :D
		mainCPU.sound_timer--;
	}
}


bool cpu_isDrawFlagSet() {
	if (mainCPU.drawFlag) {
		mainCPU.drawFlag = false;
		return true;
	} else {
		return false;
	}
}


void cpu_setKeys(byte key) {
	if (key == 0xFF) {
		for (int i = 0; i <= 16; i++) {
			mainCPU.key[i] = 0;
		}
	} else {
		mainCPU.key[key] = 1;
	}
}