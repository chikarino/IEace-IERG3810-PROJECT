#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_TFTLCD.h"
#include "Delay.h"

int main(void)
{
  u16 bg_start_x;
  u16 bg_start_y;

  clocktree_init();
  lcd_init();
  Delay(1000000);
  lcd_9341_setParameter();
  Delay(1000000);
  LCD_LIGHT_ON();
  Delay(1000000);

  /* Step 1：FILL screen（320×240） */
  lcd_fillRectangle(c_yellow,
                    0,   /* x_start */
                    240, /* x_width  */
                    0,   /* y_start */
                    320  /* y_height */
                    );

  /* Step 2：Calcute center/start for 50x50 rect ，Fill Blue */
  bg_start_x = (240 - 50) / 2;  /* = 95 */
  bg_start_y = (320 - 50) / 2;  /* = 135  */
  lcd_fillRectangle(c_blue, bg_start_x, 50, bg_start_y, 50);
	
  while (1) { }
}
