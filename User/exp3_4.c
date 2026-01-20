#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_TFTLCD.h"
#include "SysTick.h"

int main(void)
{
  const char *cuid_line1 = "1155208116 XIAO";
  const char *cuid_line2 = "1155203192 NG";

  clocktree_init();
  SysTick_Init(1);

  lcd_init();
  SysTick_DelayMs(200);
  lcd_9341_setParameter();
  SysTick_DelayMs(200);
  LCD_LIGHT_ON();

  /* Paint background to blue */
  lcd_fillRectangle(c_blue, 0, 240, 0, 320);

	/* start printing */
  lcd_showString(20, 260, cuid_line1, c_white, c_blue);
  lcd_showString(20, 220, cuid_line2, c_yellow, c_blue);

  while (1)
  {
    SysTick_DelayMs(1000);
  }
}
