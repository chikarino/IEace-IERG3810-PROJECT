#ifndef IERG3810_TFTLCD_H
#define IERG3810_TFTLCD_H

#include "stm32f10x.h"
typedef struct
{
  u16 LCD_REG;
  u16 LCD_RAM;
} LCD_TypeDef;

#define LCD_BASE ((u32)(0x6C000000 | 0x000007FE))
#define LCD      ((LCD_TypeDef *) LCD_BASE)

#define c_black  0x0000
#define c_white  0xFFFF
#define c_red    0xF800
#define c_green  0x07E0
#define c_blue   0x001F
#define c_yellow 0xFFE0
#define LCD_LIGHT_ON()   (GPIOB->BSRR = (1u << 0))
#define LCD_LIGHT_OFF()  (GPIOB->BRR  = (1u << 0))

void lcd_init(void);
void lcd_9341_setParameter(void);
void lcd_wr_reg(u16 regval);
void lcd_wr_data(u16 data);
void lcd_drawDot(u16 x, u16 y, u16 color);
void lcd_fillRectangle (u16 color, u16 x_start,u16 x_width, u16 y_start, u16 y_height);

void lcd_sevenSegment(u16 color, u16 bg_color,
                      u16 x_start, u16 y_start,
                      u16 digit_width, u16 digit_height,
                      u8 digit);

/* ASCII Display */
void lcd_showChar(u16 x, u16 y, u8 ascii, u16 color, u16 bg_color);
void lcd_showString(u16 x, u16 y, const char *str, u16 color, u16 bg_color);
void lcd_showCharOverlay(u16 x, u16 y, u8 ascii, u16 color);
void lcd_showStringOverlay(u16 x, u16 y, const char *str, u16 color);

/* Chinese Display */
void lcd_showChinChar(u16 x, u16 y, uint16_t unicode,
                      u16 color, u16 bg_color);
void lcd_showChinCharOverlay(u16 x, u16 y, uint16_t unicode, u16 color);
void lcd_showChinString(u16 x, u16 y, const uint16_t *text, u32 length, u16 color, u16 bg_color);
void lcd_showChinStringOverlay(u16 x, u16 y, const uint16_t *text, u32 length, u16 color);

#endif
