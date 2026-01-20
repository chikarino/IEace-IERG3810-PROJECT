#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_TFTLCD.h"
#include "Delay.h"
#include "SysTick.h"
int main(void)
{
  const u16 digit_width  = 80;
  const u16 digit_height = 160;
  const u16 digit_x      = (240 - digit_width) / 2;   /* CENTER：X  */
  const u16 digit_y      = (320 - digit_height) / 2;  /* CENTER：Y  */
  s32 digit;

  clocktree_init();
	Delay(500);
	SysTick_Init(1);            /* 1ms tick； AFTER calling clocktree_init() */
  lcd_init();
  Delay(1000000);
  lcd_9341_setParameter();  /* MADCTL = 0xC8 → 原點左下 */
  Delay(1000000);
  LCD_LIGHT_ON();
  Delay(100000);

	//BLINK 8 AS SAMPLE
	lcd_fillRectangle(c_black, 0, 240, 0, 320);
  lcd_sevenSegment(c_green, c_black,
                   digit_x, digit_y,
                   digit_width, digit_height,
                   8);
  SysTick_DelayMs(200);
	lcd_fillRectangle(c_black, 0, 240, 0, 320);
	SysTick_DelayMs(200);
  lcd_sevenSegment(c_green, c_black,
                   digit_x, digit_y,
                   digit_width, digit_height,
                   8);
  SysTick_DelayMs(200);
	lcd_fillRectangle(c_black, 0, 240, 0, 320);
	SysTick_DelayMs(200);



  while (1)
  {
    for (digit = 9; digit >= 0; digit--)
    {
      lcd_sevenSegment(c_yellow, c_black,
                       digit_x, digit_y,
                       digit_width, digit_height,
                       (u8)digit);
      SysTick_DelayMs(1000);
    }
  }
}
