#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "stm32f10x.h"


void Background_Init(void);

/* Update background (called every frame) */
void Background_Update(void);

void Background_Render(void);

#endif /* BACKGROUND_H */
