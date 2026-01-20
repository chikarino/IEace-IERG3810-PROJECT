#ifndef RESOURCE_H
#define RESOURCE_H

#include "stm32f10x.h"

#define RESOURCE_READ_PIXEL(ptr, idx)  ((ptr)[(idx)])

typedef struct
{
  u16 width;
  u16 height;
  const u16 *data;    /* Points to RGB565 pixel data (row-major, starting from top-left) */
} Bitmap16;

/* ────────────────────────── Global bitmap resources ────────────────────────── */
extern const Bitmap16 gBitmap_spaceMissiles_001;
extern const Bitmap16 gBitmap_spaceShips_001;
extern const Bitmap16 gBitmap_spaceShips_001_faceRight;
extern const Bitmap16 gBitmap_BulletDOWN;
extern const Bitmap16 gBitmap_BOSS;
extern const Bitmap16 gBitmap_Title;
extern const Bitmap16 gBitmap_smile;
extern const Bitmap16 gBitmap_angry;
extern const Bitmap16 gBitmap_Explosion_0;
extern const Bitmap16 gBitmap_Explosion_1;
extern const Bitmap16 gBitmap_Explosion_2;
extern const Bitmap16 gBitmap_Explosion_3;
extern const Bitmap16 gBitmap_Explosion_4;
extern const Bitmap16 gBitmap_Explosion_5;
extern const Bitmap16 gBitmap_Explosion_6;
extern const Bitmap16 gBitmap_Explosion_7;

/* ────────────────────────── Drawing API ────────────────────────── */
void lcd_drawBitmap16(s16 x, s16 y, const Bitmap16 *bmp);
void lcd_drawBitmap16_ColorKey(s16 x, s16 y, const Bitmap16 *bmp, 
                               u16 colorNotToPrint);
void lcd_drawBitmap16_Colorize(s16 x, s16 y, const Bitmap16 *bmp,
                               u16 new_color, u16 colorNotToPrint);
void lcd_clearBitmapArea(s16 x, s16 y, const Bitmap16 *bmp,
                         u16 bg_color);

#endif /* RESOURCE_H */
