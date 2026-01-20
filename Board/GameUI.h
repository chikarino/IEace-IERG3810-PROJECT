#ifndef GAMEUI_H
#define GAMEUI_H

#include "stm32f10x.h"

// UI system API
void GameUI_Init(void);
void GameUI_Render(void);
void GameUI_SetScore(u32 score);
void GameUI_SetHP(u16 current, u16 max);

#endif /* GAMEUI_H */
