#ifndef __PLAY_H
#define __PLAY_H

#include "stm32f10x.h"
#include "PS2Keyboard.h"

#define PLAYER_MOVE_SPEED 2u    

void Play_Init(void);
void Play_OnEnter(void);
void Play_UpdateLogic(void);
void Play_Render(void);
void Play_HandleEvent(const PS2KeyEvent *evt);
void Play_HandleJoyPad(u8 joypadState);

// 測試函數:生成敵機
void Play_SpawnTestEnemy(void);

// Collision system interface
void Play_GetPlayerPos(s16 *x, s16 *y);
void Play_GetPlayerSize(u16 *w, u16 *h);
void Play_PlayerTakeDamage(u16 damage);

// Score system interface
void Play_AddScore(u32 points);
u32 Play_GetScore(void);
u16 Play_GetHP(void);

#endif
