// miniProjectMain.c
#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_LED.h"
#include "IERG3810_KEY.h"
#include "IERG3810_Buzzer.h"
#include "IERG3810_EXTI.h"
#include "IERG3810_NVIC.h"
#include "IERG3810_USART.h"
#include "IERG3810_TFTLCD.h"
#include "SysTick.h"
#include "Delay.h"
#include "GameState.h"
#include "Menu.h"
#include "BootScreen.h"
#include "PS2Keyboard.h"
#include "Play.h"
#include "GameOver.h"
#include "JoyPad.h"

enum { //Alias name for Heartbeat array slots 0 to 3
    HB_SLOT_INPUT = 0,
    HB_SLOT_LOGIC,
    HB_SLOT_RENDER,
    HB_SLOT_COUNT_IN_USE
};

void EXTI15_10_IRQHandler(void)
{
    if ((EXTI->PR & (1 << 11)) != 0) {
        PS2_OnClockFallingEdge();
    }
}

static void App_Init(void)
{
    clocktree_init();
    SysTick_Init(1);
    Heartbeat_Init(10);

    IERG3810_LED_Init();
    IERG3810_KEY_Init();
    IERG3810_Buzzer_Init();
    lcd_init();
    lcd_9341_setParameter();
    LCD_LIGHT_ON();

    nvic_setPriorityGroup(5);
    key2_extiInit(0x65);
    keyup_extiInit(0x75);
    PS2_Init();
    JoyPad_Init();

    DS0_OFF;
    DS1_OFF;

    BootScreen_Init();
    Menu_Init();
    GameState_Init();
    GameState_Set(STATE_BOOT);

    /*IF WANT TO CHANGE RATE, CHANGE RATE IN THE THREE TASK_* ALSO.*/
    Heartbeat_Start(HB_SLOT_INPUT,  Heartbeat_MsToTicks(10)); //input polling every 10ms
    Heartbeat_Start(HB_SLOT_LOGIC,  Heartbeat_MsToTicks(10)); //game logic update every 10ms
    Heartbeat_Start(HB_SLOT_RENDER, Heartbeat_MsToTicks(33)); // ~30 FPS
}

static void Task_Input(void) //Called repeatedly in main loop to handle user input
{
    PS2KeyEvent evt;
	u8 joypadState;
    if (!Heartbeat_Expired(HB_SLOT_INPUT)) return; //handle input every 10ms
    Heartbeat_Restart(HB_SLOT_INPUT, Heartbeat_MsToTicks(10));

    PS2_Update();

    // Poll JoyPad
    joypadState = JoyPad_Read();
    switch (GameState_Get()) {
        case STATE_MENU: {
            MenuCommand cmd = Menu_HandleJoyPad(joypadState);
            if (cmd == MENU_CMD_START_GAME) {
                GameState_Set(STATE_PLAY);
                Play_Init();
            }
            break;
        }
        case STATE_PLAY:
            Play_HandleJoyPad(joypadState);
            break;
        case STATE_GAMEOVER:
            GameOver_HandleJoyPad(joypadState);
            break;
        default:
            break;
    }

    while (PS2_Poll(&evt)) {
        switch (GameState_Get()) {
        case STATE_MENU: {
            MenuCommand cmd = Menu_HandleEvent(&evt);
            if (cmd == MENU_CMD_START_GAME) {
                GameState_Set(STATE_PLAY);
                /* Initialize game flow here */
                Play_Init();
            }
            break;
        }
        case STATE_PLAY:
            /* Handle Keypad input in play state */
            Play_HandleEvent(&evt);
            break;
        case STATE_GAMEOVER:
            GameOver_HandleEvent(&evt);
            break;
        default:
            break;
        }
    }
}

static void Task_Logic(void) //Called repeatedly in main loop to update game logic
{
    if (!Heartbeat_Expired(HB_SLOT_LOGIC)) return;
    Heartbeat_Restart(HB_SLOT_LOGIC, Heartbeat_MsToTicks(10));

    GameState_Update();
}

static void Task_Render(void) //Called repeatedly in main loop to render the current game state
{
    if (!Heartbeat_Expired(HB_SLOT_RENDER)) return;
    Heartbeat_Restart(HB_SLOT_RENDER, Heartbeat_MsToTicks(33));

    GameState_Render();
}

int main(void)
{
    App_Init();
    while (1) {
        Task_Input(); //handle user input based on current game state. In each state, different input handling functions are called.
        Task_Logic(); //update game logic based on current game state. Each state has its own update function.
        Task_Render(); //render the current game state to the display. Each state has its own render function.
    }
}

