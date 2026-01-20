// Menu.h
#ifndef __MENU_H
#define __MENU_H

#include "stm32f10x.h"
#include "PS2Keyboard.h"

typedef enum {
    MENU_CMD_NONE = 0,
    MENU_CMD_START_GAME,
    MENU_CMD_EXIT_MENU
} MenuCommand;

void Menu_Init(void);
void Menu_OnEnter(void);
void Menu_UpdateLogic(void);
void Menu_Render(void);
void Menu_Animation(void);
MenuCommand Menu_HandleEvent(const PS2KeyEvent *evt);
MenuCommand Menu_HandleJoyPad(u8 joypadState);

u8 Menu_GetDifficulty(void);
u16 Menu_GetMaxHP(void);

#endif
