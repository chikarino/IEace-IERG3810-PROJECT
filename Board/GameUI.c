#include "GameUI.h"
#include "IERG3810_TFTLCD.h"
#include "Enemy.h"
#include <stdio.h>

#define LCD_WIDTH   240u
#define LCD_HEIGHT  320u

static u32 g_currentScore = 0u;
static u16 g_currentHP = 0u;
static u16 g_maxHP = 5u;

void GameUI_Init(void)
{
    g_currentScore = 0u;
    g_currentHP = 0u;
    g_maxHP = 5u;
}

void GameUI_SetScore(u32 score)
{
    g_currentScore = score;
}

void GameUI_SetHP(u16 hp, u16 maxHP)
{
    g_currentHP = hp;
    g_maxHP = maxHP;
}

static void GameUI_DrawScore(void)
{
    char scoreText[32];
    sprintf(scoreText, "SCORE:%05lu", (unsigned long)g_currentScore);
    lcd_showString(5, 305, scoreText, c_yellow, c_black);
}

static void GameUI_DrawHP(void)
{
    char hpText[16];
    sprintf(hpText, "HP:%u", g_currentHP);
    lcd_showString(5, 5, hpText, c_white, c_black);
}

static void GameUI_DrawBossHPBar(void)
{
    u16 bossHP;
    u16 bossMax;
    u16 filledWidth;
    const u16 barY = 335u;
    const u16 barHeight = 5u;
    const u16 barWidth = 240u;
    u8 active;
    static u8 wasActive = 0;
    
    active = Enemy_GetBossHP(&bossHP, &bossMax);
    if (!active || bossMax == 0u) {
        if (wasActive) { // Clear bar when boss defeated
            lcd_fillRectangle(c_black, 0u, barWidth, barY, barHeight);
            wasActive = 0;
        }
        return;
    }
    wasActive = 1;

    // Draw background bar
    lcd_fillRectangle(c_red, 0u, barWidth, barY, barHeight);
    
    // Calculate foreground width based on HP ratio
    filledWidth = (u16)(((u32)bossHP * barWidth) / bossMax);
    if (filledWidth > barWidth) {
        filledWidth = barWidth;
    }
    if (filledWidth > 0u) {
        lcd_fillRectangle(c_green, 0u, filledWidth, barY, barHeight);
    }
}

void GameUI_Render(void)
{
    GameUI_DrawScore();
    GameUI_DrawHP();
    GameUI_DrawBossHPBar();
}
