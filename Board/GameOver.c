#include "GameOver.h"
#include "IERG3810_TFTLCD.h"
#include "GameState.h"
#include "Resource.h"

#define LCD_WIDTH   240u
#define LCD_HEIGHT  320u

static u32 g_finalScore = 0u;
static u8 g_initialized = 0u;
static u8 g_isWin = 0u;

void GameOver_Init(void)
{
    g_finalScore = 0u;
    g_initialized = 0u;
    g_isWin = 0u;
}

void GameOver_SetMissionStatus(u8 isWin)
{
    g_isWin = isWin;
}

/* Set final score and initialize */
void GameOver_OnEnter(u32 finalScore)
{
    g_finalScore = finalScore;
    g_initialized = 0u;
}

/* Update logic */
void GameOver_UpdateLogic(void)
{
    /* No update logic needed currently */
}

/* Render screen */
void GameOver_Render(void)
{
    char scoreText[32];
    u16 i;
    
    if (!g_initialized) {
        /* Clear screen */
        lcd_fillRectangle(c_black, 0, LCD_WIDTH, 0, LCD_HEIGHT);
        g_initialized = 1u;
    }
    
    /* Display Title based on status */
    if (g_isWin) {
        lcd_showString(40, 280, "MISSION COMPLETED", c_green, c_black);
        lcd_drawBitmap16_ColorKey(LCD_WIDTH-40, 260, &gBitmap_smile, c_white);
    } else {
        lcd_showString(70, 280, "GAME OVER", c_red, c_black);
        lcd_drawBitmap16_ColorKey(LCD_WIDTH-50, 260, &gBitmap_angry, c_white);
    }
    
    /* Display final score */
    lcd_showString(60, 240, "FINAL SCORE:", c_white, c_black);
    
    /* Convert score to string and display */
    for (i = 0; i < 32; i++) {
        scoreText[i] = '\0';
    }
    
    /* Simple integer to string conversion */
    {
        u32 num = g_finalScore;
        u8 digits[10];
        u8 digitCount = 0;
        
        if (num == 0) {
            scoreText[0] = '0';
            scoreText[1] = '\0';
        } else {
            /* 提取各位數字 */
            while (num > 0 && digitCount < 10) {
                digits[digitCount] = (u8)(num % 10);
                num /= 10;
                digitCount++;
            }
            
            /* 反轉順序寫入字串 */
            for (i = 0; i < digitCount; i++) {
                scoreText[i] = '0' + digits[digitCount - 1 - i];
            }
            scoreText[digitCount] = '\0';
        }
    }
    
    lcd_showString(100, 200, scoreText, c_yellow, c_black);
    lcd_showString(40, 100, "Press KEY0/A to", c_white, c_black);
    lcd_showString(40, 80, "return to menu", c_white, c_black);
}

void GameOver_HandleEvent(const PS2KeyEvent *evt){
    u8 pressed = (evt->isBreak == 0u);
    if (!pressed) return;

    switch (evt->code) {
    case PS2_SCANCODE_KP0:
        // Return to main menu
        GameState_Set(STATE_MENU);
        break;
    default:
        break;
    }
}

void GameOver_HandleJoyPad(u8 joypadState)
{
    static u8 lastJoypadState = 0;
    u8 changed = joypadState ^ lastJoypadState;
    u8 pressed = changed & joypadState;
    lastJoypadState = joypadState;

    if (pressed & (JOYPAD_A)) {
        // Return to main menu
        GameState_Set(STATE_MENU);
    }
}
