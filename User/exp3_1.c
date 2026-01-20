#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_TFTLCD.h" // x240 y340
#include "Delay.h"
int main(void)
{
  const u16 x_pos[6] = {10, 20, 30, 40, 50, 60};
  const u16 colors[6] = {c_black, c_white, c_green, c_red, c_blue, c_yellow};
  u16 line, dot;
	u16 x, y = 0;
  clocktree_init();                /* SYSCLK 72 MHz, APB1 36 MHz, APB2 72 MHz */
  lcd_init();                      /* GPIO+FSMC+ILI9341 init */
   Delay(1000000);   
	lcd_9341_setParameter();         /* ILI9341 init: Exit Sleep Mode, Pixel Format Set,  Memory Access Control */
   Delay(1000000); 
   LCD_LIGHT_ON();
   Delay(1000000);
	//draw lines in different colours
  for (line = 0; line < 6; line++) {
    for (dot = 0; dot < 100; dot++) {
      lcd_drawDot(x_pos[line], (u16)(10 + dot), colors[line]);
      Delay(50000);
    }
  }
	for (x = 0; x < 240; x++) {
		lcd_drawDot(x, 0, c_black);
		Delay(50000);
	}


  while (1) {
	//NOP
   }
  }

