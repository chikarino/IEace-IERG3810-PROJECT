#ifndef GAMEOVER_H
#define GAMEOVER_H

#include "stm32f10x.h"
#include "PS2Keyboard.h"
#include "JoyPad.h"
#define PS2_SCANCODE_KP0    0x70
#define PS2_SCANCODE_KP1    0x69
#define PS2_SCANCODE_KP2    0x72
#define PS2_SCANCODE_KP3    0x7A
#define PS2_SCANCODE_KP5    0x73
#define PS2_SCANCODE_KP7    0x6C
#define PS2_SCANCODE_KP8    0x75
#define PS2_SCANCODE_KP9    0x7D



void GameOver_Init(void);

/* Enter GameOver screen (set final score) */
void GameOver_OnEnter(u32 finalScore);

/* Set mission status (0: Failed/Player Died, 1: Completed/Boss Defeated) */
void GameOver_SetMissionStatus(u8 isWin);

/* Update logic (Nothing currently) */
void GameOver_UpdateLogic(void);

/* Render screen */
void GameOver_Render(void);

void GameOver_HandleEvent(const PS2KeyEvent *evt);
void GameOver_HandleJoyPad(u8 joypadState);


#endif
