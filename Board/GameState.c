// GameState.c
#include "GameState.h"
#include "Menu.h"
#include "BootScreen.h"
#include "stm32f10x.h"
#include "Play.h"
#include "GameOver.h"

static GameState g_state;
static GameState g_prev;

void GameState_Init(void)
{
    g_state = STATE_BOOT;
    g_prev  = STATE_BOOT;
    BootScreen_OnEnter();
}

void GameState_Set(GameState s)
{
    if (g_state != s) {
        g_prev = g_state;
        g_state = s;

        switch (g_state) {
        case STATE_BOOT:
            BootScreen_OnEnter();
            break;
        case STATE_MENU:
            Menu_OnEnter();
            break;
        case STATE_PLAY:
            Play_OnEnter();
            break;
        case STATE_GAMEOVER:
            GameOver_OnEnter(Play_GetScore());
            break;
        default:
            break;
        }
    }
}

GameState GameState_Get(void)
{
    return g_state;
}

int GameState_Changed(void)
{
    return g_prev != g_state;
}

/* Logic updated according to current state */
void GameState_Update(void) //This is called in 10ms intervals from main loop
{
    switch (g_state) {
    case STATE_BOOT:
        if (BootScreen_Update()) {
            GameState_Set(STATE_MENU);
        }
        break;

    case STATE_MENU:
        Menu_UpdateLogic();
        break;
    
    case STATE_TUTORIAL:
        //Abanoneded for now
        break;

    case STATE_PLAY:
        Play_UpdateLogic();
        break;

    case STATE_PAUSE:
        //Abandoneded for now
        break;

    case STATE_GAMEOVER:
        GameOver_UpdateLogic();
        break;

    default:
        break;
    }
}

/* Render according to current state */
void GameState_Render(void) //Called in main loop 10ms intervals
{
    switch (g_state) {
    case STATE_BOOT:
        BootScreen_Render();
        break;

    case STATE_MENU:
        Menu_Render();
        break;

    case STATE_TUTORIAL:
        //Abanoneded for now
        break;

    case STATE_PLAY:
        Play_Render();
        break;

    case STATE_PAUSE:
        //Abandoneded for now
        break;

    case STATE_GAMEOVER:
        GameOver_Render();
        break;

    default:
        break;
    }
}
