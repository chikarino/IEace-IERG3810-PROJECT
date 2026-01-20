#include "Stage.h"
#include "Enemy.h"
#include "stdlib.h"

/* 關卡數據 */
static StageState g_stageState;
static u16 g_frameCounter;
static u8 g_waveSpawned;          /* Current wave spawned? */
static u8 g_bossDefeated;         /* Boss defeated? */
static u16 g_bossSpawnCounter;    /* Boss spawn counter */
static u8 g_bossSpawnToggle;      /* Boss spawn direction toggle (0=LEFT_DOWN, 1=RIGHT_DOWN) */

void Stage_Init(void)
{
    g_stageState = STAGE_STATE_WAVE_1;
    g_frameCounter = 0u;
    g_waveSpawned = 0u;
    g_bossDefeated = 0u;
    g_bossSpawnCounter = 0u;
    g_bossSpawnToggle = 0u;
}

/* Reset stage */
void Stage_Reset(void)
{
    Stage_Init();
}

/* Check if there are any enemies alive on the field (excluding Boss) */
static u8 Stage_HasEnemiesAlive(void)
{
    u8 i;
    for (i = 0u; i < ENEMY_MAX_COUNT; i++) {
        const Enemy *e = Enemy_Get(i);
        if (e != NULL && e->state == ENEMY_STATE_ACTIVE && e->type != ENEMY_TYPE_BOSS) {
            return 1u;
        }
    }
    return 0u;
}

/* Spawn first wave: 3 normal enemies descending straight down */
static void Stage_SpawnWave1(void)
{
    Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 60, 330, ENEMY_MOVE_STRAIGHT_DOWN);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 120, 330, ENEMY_MOVE_STRAIGHT_DOWN);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 180, 330, ENEMY_MOVE_STRAIGHT_DOWN);
    g_waveSpawned = 1u;
}

/* Spawn second wave: 5 zigzag enemies */
static void Stage_SpawnWave2(void)
{
    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 30, 330, ENEMY_MOVE_ZIGZAG);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 90, 330, ENEMY_MOVE_ZIGZAG);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 180, 330, ENEMY_MOVE_ZIGZAG);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 260, 330, ENEMY_MOVE_ZIGZAG);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 310, 350, ENEMY_MOVE_ZIGZAG);
    g_waveSpawned = 1u;
}

/* Spawn third wave: mixed formation */
static void Stage_SpawnWave3(void)
{
    /* Top row: 3 TYPE_1 */
    Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 50, 340, ENEMY_MOVE_STRAIGHT_DOWN);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 120, 340, ENEMY_MOVE_STRAIGHT_DOWN);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 190, 340, ENEMY_MOVE_STRAIGHT_DOWN);
    
    /* Bottom row: 4 TYPE_2 zigzag */
    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 30, 350, ENEMY_MOVE_ZIGZAG);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 90, 350, ENEMY_MOVE_ZIGZAG);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 150, 350, ENEMY_MOVE_ZIGZAG);
    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 210, 350, ENEMY_MOVE_ZIGZAG);
    g_waveSpawned = 1u;
}

static void Stage_SpawnBoss(void)
{
    Enemy_Spawn(ENEMY_TYPE_BOSS, 120, 350, ENEMY_MOVE_BOSS_PATTERN);
    g_waveSpawned = 1u;
}

void Stage_Update(void)
{
    g_frameCounter++;
    
    switch (g_stageState) {
    case STAGE_STATE_WAVE_1:
        /* Spawn first wave after 3 seconds */
        if (!g_waveSpawned && g_frameCounter >= 180u) {
            g_waveSpawned = 1u;
        }
        /* Spawn enemies every 15 frames */
        if (g_waveSpawned) {
            if (g_frameCounter == 180u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 60, 330, ENEMY_MOVE_STRAIGHT_DOWN);
            } else if (g_frameCounter == 195u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 120, 330, ENEMY_MOVE_STRAIGHT_DOWN);
            } else if (g_frameCounter == 210u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 180, 330, ENEMY_MOVE_STRAIGHT_DOWN);
            }
        }
        /* Wait for all enemies to be cleared */
        if (g_waveSpawned && !Stage_HasEnemiesAlive()) {
            g_stageState = STAGE_STATE_WAVE_2;
            g_frameCounter = 0u;
            g_waveSpawned = 0u;
        }
        break;
        
    case STAGE_STATE_WAVE_2:
        /* Spawn second wave after 2 seconds */
        if (!g_waveSpawned && g_frameCounter >= 120u) {
            g_waveSpawned = 1u;
        }
        /* Spawn enemies every 15 frames */
        if (g_waveSpawned) {
            if (g_frameCounter == 120u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 30, 330, ENEMY_MOVE_ZIGZAG);
            } else if (g_frameCounter == 135u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 120, 330, ENEMY_MOVE_ZIGZAG);
            } else if (g_frameCounter == 150u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 180, 330, ENEMY_MOVE_ZIGZAG);
            } else if (g_frameCounter == 165u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 260, 330, ENEMY_MOVE_ZIGZAG);
            } else if (g_frameCounter == 180u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 310, 350, ENEMY_MOVE_ZIGZAG);
            }
        }
        /* Wait for all enemies to be cleared */
        if (g_waveSpawned && !Stage_HasEnemiesAlive()) {
            g_stageState = STAGE_STATE_WAVE_3;
            g_frameCounter = 0u;
            g_waveSpawned = 0u;
        }
        break;
        
    case STAGE_STATE_WAVE_3:
        /* Spawn third wave after 2 seconds */
        if (!g_waveSpawned && g_frameCounter >= 120u) {
            g_waveSpawned = 1u;
        }
        /* Spawn enemies every 15 frames */
        if (g_waveSpawned) {
            /* 上排: 3架TYPE_1 */
            if (g_frameCounter == 120u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 50, 340, ENEMY_MOVE_STRAIGHT_DOWN);
            } else if (g_frameCounter == 135u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 120, 340, ENEMY_MOVE_STRAIGHT_DOWN);
            } else if (g_frameCounter == 150u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 190, 340, ENEMY_MOVE_STRAIGHT_DOWN);
            }
            /* Bottom row: 4 TYPE_2 zigzag */
            else if (g_frameCounter == 165u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 30, 350, ENEMY_MOVE_ZIGZAG);
            } else if (g_frameCounter == 180u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 90, 350, ENEMY_MOVE_ZIGZAG);
            } else if (g_frameCounter == 195u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 150, 350, ENEMY_MOVE_ZIGZAG);
            } else if (g_frameCounter == 210u) {
                Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 210, 350, ENEMY_MOVE_ZIGZAG);
            }
        }
        /* Wait for all enemies to be cleared */
        if (g_waveSpawned && !Stage_HasEnemiesAlive()) {
            g_stageState = STAGE_STATE_WAVE_CLEAR;
            g_frameCounter = 0u;
            g_waveSpawned = 0u;
        }
        break;
        
    case STAGE_STATE_WAVE_CLEAR:
        /* After clearing the field for 3 seconds, the Boss appears */
        if (g_frameCounter >= 180u) {
            g_stageState = STAGE_STATE_BOSS;
            g_frameCounter = 0u;
        }
        break;
        
    case STAGE_STATE_BOSS:
        /* Spawn Boss */
        if (!g_waveSpawned) {
            Stage_SpawnBoss();
            g_bossSpawnCounter = 0u;  /* Reset spawn counter */
        }
        
        /* Spawn NORMAL_2 enemies every 210 frames during Boss stage */
        if (g_waveSpawned && !g_bossDefeated) {
            g_bossSpawnCounter++;
            if (g_bossSpawnCounter >= 210u) {
                EnemyMovePattern pattern;
                
                /* Determine whether to use LEFT_DOWN or RIGHT_DOWN based on toggle */
                pattern = g_bossSpawnToggle ? ENEMY_MOVE_RIGHT_DOWN : ENEMY_MOVE_LEFT_DOWN;
                g_bossSpawnToggle = !g_bossSpawnToggle;  /* Toggle direction */
                
                /* Spawn 3 NORMAL_2 enemies every 15 frames */
                if (g_bossSpawnCounter == 210u) {
                    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 50, 330, pattern);
                } else if (g_bossSpawnCounter == 225u) {
                    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 120, 330, pattern);
                } else if (g_bossSpawnCounter == 240u) {
                    Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 190, 330, pattern);
                    g_bossSpawnCounter = 0u;  /* Reset counter */
                }
            }
        }
        
        /* After the Boss is defeated, enter the ending animation state */
        if (g_bossDefeated) {
            g_stageState = STAGE_STATE_ENDING;
            g_frameCounter = 0u;
        }
        break;
        
    case STAGE_STATE_ENDING:
        /* Wait for 2 seconds before entering the complete state (to allow explosion animations, etc.) */
        if (g_frameCounter >= 120u) {
            g_stageState = STAGE_STATE_COMPLETE;
        }
        break;
        
    case STAGE_STATE_COMPLETE:
        /* Stage complete, can display clear screen */
        break;
        
    default:
        break;
    }
}

/* Get the current stage state */
StageState Stage_GetState(void)
{
    return g_stageState;
}

void Stage_OnBossDefeated(void)
{
    g_bossDefeated = 1u;
}
