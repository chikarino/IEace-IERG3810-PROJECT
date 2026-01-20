#ifndef COLLISION_H
#define COLLISION_H

#include "stm32f10x.h"

// Collision system API
void Collision_Init(void);
void Collision_CheckAll(void);

// Rectangle collision detection (using sprite sizes)
u8 Collision_RectRect(s16 x1, s16 y1, u16 w1, u16 h1,
                      s16 x2, s16 y2, u16 w2, u16 h2);

#endif /* COLLISION_H */
