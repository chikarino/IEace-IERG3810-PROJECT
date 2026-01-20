#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_TFTLCD.h"
#include "SysTick.h"

int main(void)
{
char *cuid = "1155208116";
char *cuid1 = "1155208116";

uint16_t name_chars[] = { 0x8096, 0x5BB6, 0x671B }; /* 肖家望 */
uint16_t name_chars1[] = { 0x5B87, 0x667a, 0x6ce2, 0x4f50,0x52a9 }; /* 宇智波佐助　*/
uint16_t name_chars2[] = { 0x5433, 0x921e, 0x705d }; /*吳鈞灝 */



  clocktree_init();
  SysTick_Init(1);
  lcd_init();
  SysTick_DelayMs(200);
  lcd_9341_setParameter();
  SysTick_DelayMs(200);
  LCD_LIGHT_ON();




lcd_fillRectangle(c_green, 0, 240, 0, 320);
lcd_fillRectangle(c_red, 0, 80, 0, 320);



/* ASCII：with BG */
lcd_showString(20, 270, cuid, c_white, c_blue);

/* ASCII：Overlay */
lcd_showStringOverlay(20, 250, cuid, c_yellow);

/* 中文：有背景 */
lcd_showChinString(20, 210, name_chars, 5, c_white, c_blue);

/* 中文：透明疊加 */
lcd_showChinStringOverlay(20, 190, name_chars1, 5, c_yellow);
lcd_showChinStringOverlay(20, 170, name_chars2, 3, c_yellow);

	

  while (1)
  {
    SysTick_DelayMs(1000);
  }

}
