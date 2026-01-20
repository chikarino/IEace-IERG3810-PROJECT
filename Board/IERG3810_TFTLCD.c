#include "IERG3810_TFTLCD.h"
#include "FONT.H"
#include "CFONT.H"
extern void Delay(u32 count);
void lcd_init(void)           // set FSMC
{
  RCC->AHBENR |= 1 << 8;      // FSMC
  RCC->APB2ENR |= 1 << 3;     // PORTB
  RCC->APB2ENR |= 1 << 5;     // PORTD
  RCC->APB2ENR |= 1 << 6;     // PORTE
  RCC->APB2ENR |= 1 << 8;     // PORTG
	
	GPIOB->CRL &= 0XFFFFFFF0;		// Clear GPIOB
  GPIOB->CRL |= 0X00000003;   // PB0 output (背光)
	//PORTD
  GPIOD->CRH &= 0X00FFF000; 
  GPIOD->CRH |= 0XBB000BBB;
  GPIOD->CRL &= 0XFF00FF00;
  GPIOD->CRL |= 0X00BB00BB;
	//PORTE
  GPIOE->CRH &= 0X00000000; 
  GPIOE->CRH |= 0XBBBBBBBB; 
  GPIOE->CRL &= 0X0FFFFFFF;  
  GPIOE->CRL |= 0XB0000000;  
	//PORTG12
  GPIOG->CRH &= 0XFFF0FFFF;
  GPIOG->CRH |= 0X000B0000;
  GPIOG->CRL &= 0XFFFFFFF0;//PG0->RS
  GPIOG->CRL |= 0X0000000B;

  // LCD uses FSMC Bank 4 memory bank.
  // Use Mode A
  FSMC_Bank1->BTCR[6] = 0X00000000;  // FSMC_BCR4 (reset)
  FSMC_Bank1->BTCR[7] = 0X00000000;  // FSMC_BTR4 (reset)
  FSMC_Bank1E->BWTR[6] = 0X00000000; // FSMC_BWTR4 (reset)
  FSMC_Bank1->BTCR[6] |= 1 << 12;    // FSMC_BCR4 -> WREN  write enable 
  FSMC_Bank1->BTCR[6] |= 1 << 14;    // FSMC_BCR4 -> EXTMOD  
  FSMC_Bank1->BTCR[6] |= 1 << 4;     // FSMC_BCR4 -> NWID		//16bit data width

  FSMC_Bank1->BTCR[7] |=  0<< 28;    // FSMC_BTR4 -> ACCMOD	//mode A
  FSMC_Bank1->BTCR[7] |= 1<<0;     // FSMC_BTR4 -> ADDSET		//read
  FSMC_Bank1->BTCR[7] |= 0XF<<8;    // FSMC_BTR4 -> DATAST
  FSMC_Bank1E->BWTR[6] |= 0<<28;   // FSMC_BWTR4 -> ACCMOD	//write
  FSMC_Bank1E->BWTR[6] |= 0<<0;    // FSMC_BWTR4 -> ADDSET	
  FSMC_Bank1E->BWTR[6] |= 3<<8;    // FSMC_BWTR4 -> DATAST
  FSMC_Bank1->BTCR[6] |= 1 << 0;     // FSMC_BCR4 -> MBKEN //memory bank 4 enable
}

void lcd_9341_setParameter(void)
{
  lcd_wr_reg(0X01);   // Software reset
  lcd_wr_reg(0X11);   // Exit_sleep mode
  lcd_wr_reg(0X3A);   //Memory data access control // COLMOD: Pixel Format Set
  lcd_wr_data(0X55);
  lcd_wr_reg(0X29);   // Display on
  lcd_wr_reg(0X36);   // Memory data access control
  lcd_wr_data(0XC8); //control display direction
}

void lcd_wr_reg(u16 regval)
{
  LCD->LCD_REG = regval;
}

void lcd_wr_data(u16 data)
{
  LCD->LCD_RAM = data;
}

void lcd_drawDot(u16 x, u16 y, u16 color)
{
  lcd_wr_reg(0x2A);           // set x position
    lcd_wr_data(x >> 8);
    lcd_wr_data(x & 0xFF);
    lcd_wr_data(0x01);
    lcd_wr_data(0x3F);
  lcd_wr_reg(0x2B);           // set y position
    lcd_wr_data(y >> 8);
    lcd_wr_data(y & 0xFF);
    lcd_wr_data(0x01);
    lcd_wr_data(0xDF);
  lcd_wr_reg(0x2C);           // set point with color
  lcd_wr_data(color);
}

void lcd_fillRectangle (u16 color, u16 x_start,u16 x_width, u16 y_start, u16 y_height)
{
  u32 index=0;
  lcd_wr_reg(0X2A);           // set x position
    lcd_wr_data(x_start >> 8);
    lcd_wr_data(x_start & 0xFF);
    lcd_wr_data((x_start + x_width - 1) >> 8);
    lcd_wr_data((x_start + x_width - 1) & 0xFF);
  lcd_wr_reg(0X2B);           // set y position
    lcd_wr_data(y_start >> 8);
    lcd_wr_data(y_start & 0xFF);
    lcd_wr_data((y_start + y_height - 1) >> 8);
    lcd_wr_data((y_start + y_height - 1) & 0xFF);
  lcd_wr_reg(0X2C);           //LCD_WriteRAM_Prepare
  Delay(100);
  for(index=0;index<x_width * y_height; index++)
  {
    lcd_wr_data(color);
  }
}

/* ──────────────────────7 SEGMENT AREA ────────────────────── */

#define SEG_A  (1u << 0)
#define SEG_B  (1u << 1)
#define SEG_C  (1u << 2)
#define SEG_D  (1u << 3)
#define SEG_E  (1u << 4)
#define SEG_F  (1u << 5)
#define SEG_G  (1u << 6)

static const u8 seven_seg_digits[10] =
{
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,           /* 0 */
  SEG_B | SEG_C,                                           /* 1 */
  SEG_A | SEG_B | SEG_D | SEG_E | SEG_G,                   /* 2 */
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_G,                   /* 3 */
  SEG_B | SEG_C | SEG_F | SEG_G,                           /* 4 */
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,                   /* 5 */
  SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,           /* 6 */
  SEG_A | SEG_B | SEG_C,                                   /* 7 */
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,   /* 8 */
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G            /* 9 */
};

/* Aux：only when width/height > 0, draw rect */
static void lcd_drawSegmentRect(u16 color,
                                u16 x_start, u16 x_width,
                                u16 y_start, u16 y_height)
{
  if ((x_width == 0) || (y_height == 0)) return;
  lcd_fillRectangle(color, x_start, x_width, y_start, y_height);
}

/*SEVEN SEGMENT */
void lcd_sevenSegment(u16 color, u16 bg_color,
                      u16 x_start, u16 y_start,
                      u16 digit_width, u16 digit_height,
                      u8 digit){

  u16 segment_thick;
  s32 vertical_space;
  u16 lower_vertical;
  u16 upper_vertical;
  u16 mid_y;
  u16 upper_start_y;
  u16 right_x;
  u8 pattern;

  if (digit > 9) return;

  /* Step 0：Clear the Background */
  lcd_fillRectangle(bg_color, x_start, digit_width, y_start, digit_height);

  /* Step 1：Calculate segment thickness and vertical space */
  // segment_thick At lease 1 pixel
  // segment_thick is 1/5 of digit_width by default
  // but if that makes segment_thick*3 >= digit_height,
  // then segment_thick is 1/6 of digit_height
  // segment_thick is at least 1 pixel
  segment_thick = digit_width / 5;
  if (segment_thick == 0) segment_thick = 1;
  if ((segment_thick * 3) >= digit_height)
  {
    segment_thick = digit_height / 6;
    if (segment_thick == 0) segment_thick = 1;
  }
	segment_thick=10; // segment thinck is 10 by manual requirement
  vertical_space = (s32)digit_height - (s32)(3 * segment_thick);
  if (vertical_space < 0) vertical_space = 0;
  lower_vertical = (u16)(vertical_space / 2);
  upper_vertical = (u16)(vertical_space - lower_vertical);

  mid_y        = y_start + lower_vertical + segment_thick;
  upper_start_y= y_start + lower_vertical + (u16)(2 * segment_thick);
  right_x      = x_start + digit_width - segment_thick;

  pattern = seven_seg_digits[digit];

  /* Step 2：Draw according to pattern */
  if (pattern & SEG_A)
  {
    lcd_drawSegmentRect(color,
                        x_start + segment_thick, digit_width - 2 * segment_thick,
                        y_start + digit_height - segment_thick,
                        segment_thick);
  }

  if (pattern & SEG_D)
  {
    lcd_drawSegmentRect(color,
                        x_start + segment_thick, digit_width - 2 * segment_thick,
                        y_start,
                        segment_thick);
  }

  if (pattern & SEG_G)
  {
    lcd_drawSegmentRect(color,
                        x_start + segment_thick, digit_width - 2 * segment_thick,
                        mid_y,
                        segment_thick);
  }

  if (pattern & SEG_F)
  {
    lcd_drawSegmentRect(color,
                        x_start, segment_thick,
                        upper_start_y,
                        upper_vertical);
  }

  if (pattern & SEG_E)
  {
    lcd_drawSegmentRect(color,
                        x_start, segment_thick,
                        y_start + segment_thick,
                        lower_vertical);
  }

  if (pattern & SEG_B)
  {
    lcd_drawSegmentRect(color,
                        right_x, segment_thick,
                        upper_start_y,
                        upper_vertical);
  }

  if (pattern & SEG_C)
  {
    lcd_drawSegmentRect(color,
                        right_x, segment_thick,
                        y_start + segment_thick,
                        lower_vertical);
  }
}

/* ────────────────────── ASCII DISPLAY ────────────────────── */
void lcd_showChar(u16 x, u16 y, u8 ascii, u16 color, u16 bg_color){
  u8 i, b, temp1, temp2;
  u16 tempX, tempY;
  if ((ascii < 32) || (ascii > 126)) return; // not valid
  ascii -= 32;
  tempX = x;
  for (i=0; i<16; i+=2){
    temp1 = asc2_1608[ascii][i];
    temp2 = asc2_1608[ascii][i+1];
    tempY = y;
    for (b=0; b<8; b++){
      if (temp1%2==1) lcd_drawDot(tempX, tempY+8, color);
				else lcd_drawDot(tempX, tempY+8, bg_color);
      if (temp2%2==1) lcd_drawDot(tempX, tempY, color);
				else lcd_drawDot(tempX, tempY, bg_color);
      temp1 = temp1>>1;
      temp2 = temp2>>1;
      tempY++;
  }
tempX++;
  }
}

void lcd_showString(u16 x, u16 y, const char *str, u16 color, u16 bg_color)
{
  u16 cursor_x = x;
  if (str == (const char *)0) return;
  while (*str != '\0')
  {
    lcd_showChar(cursor_x, y, (u8)(*str), color, bg_color);
    cursor_x += 8;          /* every word has width of 8 pixel */
    str++;
  }
}


void lcd_showStringOverlay(u16 x, u16 y, const char *str, u16 color)
{
  u16 cursor_x;

  if (str == (const char *)0) return;

  cursor_x = x;
  while (*str != '\0')
  {
    lcd_showCharOverlay(cursor_x, y, (u8)(*str), color);
    cursor_x += 8u;
    str++;
  }
}
void lcd_showCharOverlay (u16 x, u16 y, u8 ascii, u16 color){
  u8 i, b, temp1, temp2;
  u16 tempX, tempY;
  if ((ascii < 32) || (ascii > 127)) return; // not valid
  ascii -= 32;
  tempX = x;
  for (i=0; i<16; i+=2){
    temp1 = asc2_1608[ascii][i];
    temp2 = asc2_1608[ascii][i+1];
    tempY = y;
    for (b=0; b<8; b++){
      if (temp1%2==1) lcd_drawDot(tempX, tempY+8, color);
				//else lcd_drawDot(tempX, tempY+8, bg_color);
      if (temp2%2==1) lcd_drawDot(tempX, tempY, color);
				//else lcd_drawDot(tempX, tempY, bg_color);
      temp1 = temp1>>1;
      temp2 = temp2>>1;
      tempY++;
  }
tempX++;
  }
}



/* ────────────────────── CHINESE DISPLAY ────────────────────── */
static const uint8_t *chi1616_findData(uint16_t unicode) //unicode lookup
{
  uint32_t i;
  for (i = 0u; i < CH_FONT_COUNT; i++) {
    if (chi_1616[i].unicode == unicode) {
      return chi_1616[i].data;
    }
  }
  return (const uint8_t *)0;
}

void lcd_showChinChar(u16 x, u16 y, uint16_t unicode, u16 color, u16 bg_color)
{
  const uint8_t *glyph; 
  u16 col, row;
  uint8_t byteH, byteL;

  glyph = chi1616_findData(unicode);
  if (glyph == (const uint8_t *)0) return;

  for (col = 0; col < 16; col++)
  {
    byteH = glyph[col * 2];        /* y+8 ~ y+15 */
    byteL = glyph[col * 2 + 1];   /* y   ~ y+7  */

    for (row = 0; row < 8; row++)
    {
      if (byteL & 0x80) //check highest bit
        lcd_drawDot(x + col, y + row, color);
      else
        lcd_drawDot(x + col, y + row, bg_color);
      byteL <<= 1;
    }

    for (row = 0u; row < 8; row++)
    {
      if (byteH & 0x80) //check highest bit
        lcd_drawDot(x + col, y + 8 + row, color);
      else
        lcd_drawDot(x + col, y + 8 + row, bg_color);
      byteH <<= 1;
    }
  }
}


void lcd_showChinCharOverlay(u16 x, u16 y, uint16_t unicode,u16 color)
{
  const uint8_t *glyph;
  u16 col, row;
  uint8_t byteH, byteL;

  glyph = chi1616_findData(unicode);
  if (glyph == (const uint8_t *)0) return;

  for (col = 0; col < 16; col++)
  {
    byteH = glyph[col * 2];        /* y+8 ~ y+15 */
    byteL = glyph[col * 2 + 1];   /* y   ~ y+7  */

    for (row = 0; row < 8; row++)
    {
      if (byteL & 0x80u)
        lcd_drawDot(x + col, y + row, color);
     // else lcd_drawDot(x + col, y + row, bg_color); // not print bg_color
      byteL <<= 1;
    }

    for (row = 0; row < 8; row++)
    {
      if (byteH & 0x80)
        lcd_drawDot(x + col, y + 8 + row, color);
      // else  lcd_drawDot(x + col, y + 8u + row, bg_color); // not print bg_color
      byteH <<= 1;
    }
  }
}
void lcd_showChinString(u16 x, u16 y, const uint16_t *text, u32 length, u16 color, u16 bg_color)
{
  u32 i;
  u16 cursor_x;

  if ((text == (const uint16_t *)0) || (length == 0u)) return;

  cursor_x = x;
  for (i = 0u; i < length; i++)
  {
    lcd_showChinChar(cursor_x, y, text[i], color, bg_color);
    cursor_x += 16u;
  }
}

void lcd_showChinStringOverlay(u16 x, u16 y, const uint16_t *text, u32 length, u16 color)
{
  u32 i;
  u16 cursor_x;

  if ((text == (const uint16_t *)0) || (length == 0u)) return;

  cursor_x = x;
  for (i = 0u; i < length; i++)
  {
    lcd_showChinCharOverlay(cursor_x, y, text[i], color);
    cursor_x += 16u;
  }
}
