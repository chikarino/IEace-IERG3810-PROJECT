#include "Background.h"
#include "IERG3810_TFTLCD.h"
#include <stdlib.h>

#define LCD_WIDTH   240u
#define LCD_HEIGHT  320u
#define STAR_COUNT 60

typedef struct {
    s16 x;
    s16 y;
    u8 speed;
    u16 color;
    u8 brightness;
} Star;


static Star g_stars[STAR_COUNT]; //Star pool
static const u16 STAR_COLORS[3] = {
    0x4208,  /* Dark gray */
    0x8410,  /* Medium gray */
    0xFFFF   /* Bright white */
};

void Background_Init(void)
{
    u16 i;
    srand(1234);
    
    /* Initialize all stars */
    for (i = 0; i < STAR_COUNT; i++) {
        g_stars[i].x = (s16)(rand() % LCD_WIDTH);
        g_stars[i].y = (s16)(rand() % LCD_HEIGHT);
        g_stars[i].speed = (u8)(1 + (rand() % 3)); // Speed 1-3
        g_stars[i].brightness = (u8)(rand() % 3); // Brightness 0-2
        g_stars[i].color = STAR_COLORS[g_stars[i].brightness];
    }
}
void Background_Update(void) //After initialization of stars pool
{
    u16 i;
    
    for (i = 0; i < STAR_COUNT; i++) {
        Star *star = &g_stars[i];
        
        /* Clear old position (draw black dot) */
        lcd_drawDot((u16)star->x, (u16)star->y, c_black);
        
        /* Update position (move down) */
        star->y -= star->speed;
        
        /* Loop back to top */
        if (star->y < 0) {
            star->y = LCD_HEIGHT - 1;
            star->x = (s16)(rand() % LCD_WIDTH);  /* Random X position */
            
            /* Occasionally change brightness */
            if ((rand() % 10) < 3) {
                star->brightness = (u8)(rand() % 3);
                star->color = STAR_COLORS[star->brightness];
            }
        }
    }
}

/* Render background */
void Background_Render(void)
{
    u16 i;
    /* Draw all stars */
    for (i = 0; i < STAR_COUNT; i++) {
        const Star *star = &g_stars[i];
        /* Draw star dot */
        lcd_drawDot((u16)star->x, (u16)star->y, star->color);
    }
}
