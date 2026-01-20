#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "stm32f10x.h"

/* Explosion pool size */
#define EXPLOSION_MAX_COUNT 10

/* Number of animation frames */
#define EXPLOSION_FRAME_COUNT 8

/* Explosion state */
typedef struct {
    u8 active;           /* Whether slot is in use */
    s16 x;               /* Center X coordinate */
    s16 y;               /* Center Y coordinate */
    u8 currentFrame;     /* Current animation frame (0-7) */
    u8 frameTimer;       /* Frame timer */
    u8 frameDelay;       /* Per-frame delay (frames) */
    u8 startDelay;       /* Start delay (frames before playback) */
} Explosion;

/* Initialize the explosion system */
void Explosion_Init(void);

/* Spawn an immediate explosion */
void Explosion_Spawn(s16 x, s16 y);

/* Spawn a delayed explosion */
void Explosion_SpawnDelayed(s16 x, s16 y, u8 delayFrames);

/* Update all explosions */
void Explosion_UpdateAll(void);

/* Render all explosions */
void Explosion_RenderAll(void);

#endif /* EXPLOSION_H */
