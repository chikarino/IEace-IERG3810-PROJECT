#ifndef JOYPAD_H
#define JOYPAD_H
#include "stm32f10x.h"

#define JOYPAD_A      (1 << 0)
#define JOYPAD_B      (1 << 1)
#define JOYPAD_START  (1 << 2)
#define JOYPAD_SELECT (1 << 3)
#define JOYPAD_UP     (1 << 4)
#define JOYPAD_DOWN   (1 << 5)
#define JOYPAD_LEFT   (1 << 6)
#define JOYPAD_RIGHT  (1 << 7)

void JoyPad_Init(void);
u8 JoyPad_Read(void); // Returns status byte, 1 means pressed (inverted internally for convenience)

#endif
