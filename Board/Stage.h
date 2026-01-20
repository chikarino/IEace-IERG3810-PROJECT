#ifndef STAGE_H
#define STAGE_H

#include "stm32f10x.h"

/* Stage states */
typedef enum {
    STAGE_STATE_WAVE_1,
    STAGE_STATE_WAVE_2,
    STAGE_STATE_WAVE_3,
    STAGE_STATE_WAVE_CLEAR,  /* Clear (waiting for all enemies to be destroyed/leave) */
    STAGE_STATE_BOSS,        /* Boss battle */
    STAGE_STATE_ENDING,      /* Ending animation (2 seconds delay) */
    STAGE_STATE_COMPLETE     /* Stage complete */
} StageState;

void Stage_Init(void);

/* Reset stage (restart) */
void Stage_Reset(void);

/* Update stage logic (called every 10ms) */
void Stage_Update(void);
    
/* Get current stage state */
StageState Stage_GetState(void);

/* Called when boss is defeated */
void Stage_OnBossDefeated(void);

#endif
