//
//  CPU.h
//  CHIP8
//
//  Created by Valtteri Koskivuori on 29/07/16.
//  Copyright © 2016 Valtteri Koskivuori. All rights reserved.
//

#ifndef CPU_h
#define CPU_h

#include "cpuObj.h"

void cpu_initialize();
int cpu_loadGame(char *filepath);
void cpu_emulateCycle();
void getCurrentFrame(char *buf, int count);
bool cpu_isDrawFlagSet();
void cpu_setKeys(byte key);


#endif /* CPU_h */
