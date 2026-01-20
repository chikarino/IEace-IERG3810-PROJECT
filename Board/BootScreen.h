#ifndef BOOTSCREEN_H
#define BOOTSCREEN_H

#include "stm32f10x.h"

void BootScreen_Init(void);
void BootScreen_OnEnter(void);
int  BootScreen_Update(void);
void BootScreen_Render(void);

#endif
