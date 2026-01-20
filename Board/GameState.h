#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "stm32f10x.h"

typedef enum {
    STATE_BOOT,
    STATE_MENU,
    STATE_TUTORIAL,
    STATE_PLAY,
    STATE_PAUSE,
    STATE_GAMEOVER
} GameState;

void GameState_Init(void);
void GameState_Set(GameState s);
GameState GameState_Get(void);
int  GameState_Changed(void);
void GameState_Update(void);
void GameState_Render(void);

#endif
