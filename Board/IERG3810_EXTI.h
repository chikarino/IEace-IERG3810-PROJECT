#ifndef IERG3810_EXTI_H
#define IERG3810_EXTI_H
#include "stm32f10x.h"


void key2_extiInit(u8 priority);
void keyup_extiInit(u8 priority);
void ps2key_extiInit(u8 priority);

#endif
