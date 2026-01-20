#include "Explosion.h"
#include "Resource.h"
#include "IERG3810_TFTLCD.h"
#include <stdlib.h>

#define EXPLOSION_TRANSPARENT_COLOR 0x0000

/* Explosion object pool */
static Explosion g_explosionPool[EXPLOSION_MAX_COUNT];

/* Explosion animation sprites */
static const Bitmap16* g_explosionSprites[EXPLOSION_FRAME_COUNT] = {
    &gBitmap_Explosion_0,
    &gBitmap_Explosion_1,
    &gBitmap_Explosion_2,
    &gBitmap_Explosion_3,
    &gBitmap_Explosion_4,
    &gBitmap_Explosion_5,
    &gBitmap_Explosion_6,
    &gBitmap_Explosion_7
};

/* Initialize explosion system */
void Explosion_Init(void)
{
    u8 i;
    for (i = 0; i < EXPLOSION_MAX_COUNT; i++) {
        g_explosionPool[i].active = 0;
        g_explosionPool[i].x = 0;
        g_explosionPool[i].y = 0;
        g_explosionPool[i].currentFrame = 0;
        g_explosionPool[i].frameTimer = 0;
        g_explosionPool[i].frameDelay = 3;  /* Each frame lasts 3 ticks (~50 ms) */
        g_explosionPool[i].startDelay = 0;  /* No delay */
    }
}

    /* Acquire a free explosion slot */
static Explosion* Explosion_GetFreeSlot(void)
{
    u8 i;
    for (i = 0; i < EXPLOSION_MAX_COUNT; i++) {
        if (!g_explosionPool[i].active) {
            return &g_explosionPool[i];
        }
    }
    return NULL;
}

/* Spawn an explosion immediately */
void Explosion_Spawn(s16 x, s16 y)
{
    Explosion* exp;
    
    exp = Explosion_GetFreeSlot();
    if (exp == NULL) return;  /* Pool is full */
    
    exp->active = 1;
    exp->x = x;
    exp->y = y;
    exp->currentFrame = 0;
    exp->frameTimer = 0;
    exp->frameDelay = 3;  /* Each frame lasts 3 ticks */
    exp->startDelay = 0;  /* No delay, play immediately */
}

/* Spawn an explosion with a delay */
void Explosion_SpawnDelayed(s16 x, s16 y, u8 delayFrames)
{
    Explosion* exp;
    
    exp = Explosion_GetFreeSlot();
    if (exp == NULL) return;  /* Pool is full */
    
    exp->active = 1;
    exp->x = x;
    exp->y = y;
    exp->currentFrame = 0;  /* Not started yet */
    exp->frameTimer = 0;
    exp->frameDelay = 3;  /* Each frame lasts 3 ticks */
    exp->startDelay = delayFrames;  /* Delay before playback */
}

/* Clear the current explosion frame */
static void Explosion_Clear(Explosion* exp)
{
    const Bitmap16* sprite;
    s16 drawX, drawY;
    
    if (exp->currentFrame >= EXPLOSION_FRAME_COUNT) return;
    
    sprite = g_explosionSprites[exp->currentFrame];
    
    /* Compute top-left coordinate (center aligned) */
    drawX = exp->x - (s16)(sprite->width / 2);
    drawY = exp->y - (s16)(sprite->height / 2);
    
    lcd_clearBitmapArea(drawX, drawY, sprite, c_black);
}

/* Update all explosions */
void Explosion_UpdateAll(void)
{
    u8 i;
    
    for (i = 0; i < EXPLOSION_MAX_COUNT; i++) {
        Explosion* exp = &g_explosionPool[i];
        
        if (!exp->active) continue;
        
        /* Countdown the start delay if needed */
        if (exp->startDelay > 0) {
            exp->startDelay--;
            continue;  /* Skip this frame */
        }
        
        /* Advance the frame timer */
        exp->frameTimer++;
        
        if (exp->frameTimer >= exp->frameDelay) {
            /* Clear the current frame */
            Explosion_Clear(exp);
            
            /* Switch to the next frame */
            exp->frameTimer = 0;
            exp->currentFrame++;
            
            /* Animation finished */
            if (exp->currentFrame >= EXPLOSION_FRAME_COUNT) {
                exp->active = 0;
            }
        }
    }
}

/* Render all explosions */
void Explosion_RenderAll(void)
{
    u8 i;
    
    for (i = 0; i < EXPLOSION_MAX_COUNT; i++) {
        Explosion* exp;
        const Bitmap16* sprite;
        s16 drawX, drawY;
        
        exp = &g_explosionPool[i];
        
        if (!exp->active) continue;
        if (exp->startDelay > 0) continue;  /* Still delaying, nothing to draw */
        if (exp->currentFrame >= EXPLOSION_FRAME_COUNT) continue;
        
        /* Fetch the sprite for the current frame */
        sprite = g_explosionSprites[exp->currentFrame];
        
        /* Compute top-left coordinate (center aligned) */
        drawX = exp->x - (s16)(sprite->width / 2);
        drawY = exp->y - (s16)(sprite->height / 2);
        
        /* Draw with color-keying (treat 0x0000 as transparent) */
        lcd_drawBitmap16_ColorKey(drawX, drawY, sprite, EXPLOSION_TRANSPARENT_COLOR);
    }
}
