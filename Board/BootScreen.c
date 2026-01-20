#include "BootScreen.h"
#include "IERG3810_TFTLCD.h"
#include "SysTick.h"
#include "Resource.h"

#define BOOTSCREEN_DURATION_MS 500u

static u32 bootStartTick = 0u;
static u8  bootDirty     = 1u;

void BootScreen_Init(void)
{
    bootStartTick = 0u;
    bootDirty     = 1u;
}

void BootScreen_OnEnter(void)
{
    bootStartTick = SysTick_GetTick();
    bootDirty     = 1u;
}

int BootScreen_Update(void)
{
    u32 elapsed = SysTick_GetTick() - bootStartTick;
    return (elapsed >= BOOTSCREEN_DURATION_MS) ? 1 : 0;
}

void BootScreen_Render(void)
{
    if (!bootDirty) return;
    bootDirty = 0u;

    lcd_fillRectangle(c_black, 0, 240, 0, 320);
    lcd_showString(30, 250, "Thunder Fighter!", c_white, c_black);
    lcd_showString(30, 210, "Preparing systems...", c_white, c_black);
    lcd_showString(30, 170, "Entering menu shortly", c_white, c_black);
    lcd_drawBitmap16(10, 5, &gBitmap_Title);
}
