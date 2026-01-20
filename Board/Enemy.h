#ifndef ENEMY_H
#define ENEMY_H

#include "stm32f10x.h"
#include "Resource.h"

// Enemy type definitions
typedef enum {
    ENEMY_TYPE_NORMAL_1,     // Standard enemy 1 (straight descent)
    ENEMY_TYPE_NORMAL_2,     // Standard enemy 2 (zigzag)
    ENEMY_TYPE_BOSS,         // Boss
    ENEMY_TYPE_COUNT
} EnemyType;

// Enemy movement patterns
typedef enum {
    ENEMY_MOVE_STRAIGHT_DOWN,    // Straight descent
    ENEMY_MOVE_ZIGZAG,           // Zigzag path
    ENEMY_MOVE_LEFT_DOWN,        // Down-left
    ENEMY_MOVE_RIGHT_DOWN,       // Down-right
    ENEMY_MOVE_BOSS_PATTERN,     // Boss-exclusive pattern
    ENEMY_MOVE_COUNT
} EnemyMovePattern;

// Enemy states
typedef enum {
    ENEMY_STATE_INACTIVE,   // Inactive
    ENEMY_STATE_ACTIVE,     // Active
    ENEMY_STATE_DYING,      // Playing death animation
    ENEMY_STATE_DEAD        // Dead (waiting for reuse)
} EnemyState;

// Enemy structure
typedef struct {
    EnemyState state;
    EnemyType type;
    EnemyMovePattern movePattern;
    
    s16 x, y;               // Current position
    s16 prev_x, prev_y;     // Previous position (for clearing)
    s16 vx, vy;             // Velocity vector
    
    u16 hp;                 // Current HP
    u16 maxHp;              // Maximum HP
    u16 score;              // Score reward
    
    u16 frameCounter;       // Frame counter (animation/behaviour)
    u16 shootTimer;         // Shooting timer
    u16 shootInterval;      // Shooting interval (frames)
    u16 hitFlashTimer;      // Hit-flash timer
    
    const Bitmap16 *sprite; // Sprite
    u16 spriteColor;        // Tint color (when colorized)
    
    // Movement-specific parameters
    s16 spawnX, spawnY;     // Spawn position
    u16 moveParam1;         // Movement parameter 1 (amplitude, turning point, etc.)
    u16 moveParam2;         // Movement parameter 2 (frequency, etc.)
    
    u8 canShoot;            // Whether the enemy can shoot
    u8 renderNeeded;        // Whether a redraw is required
} Enemy;

// Enemy pool size
#define MAX_ENEMIES 20
#define ENEMY_MAX_COUNT MAX_ENEMIES

// Enemy system API
void Enemy_Init(void);
void Enemy_UpdateAll(void);
void Enemy_RenderAll(void);

Enemy* Enemy_Spawn(EnemyType type, s16 x, s16 y, EnemyMovePattern pattern);
void Enemy_Destroy(Enemy *enemy);
void Enemy_TakeDamage(Enemy *enemy, u16 damage);

u8 Enemy_IsActive(const Enemy *enemy);
Enemy* Enemy_GetPool(void);
u16 Enemy_GetActiveCount(void);
const Enemy* Enemy_Get(u8 index);
u8 Enemy_GetBossHP(u16 *current, u16 *max);

// Enemy configuration helpers
u16 Enemy_GetWidth(EnemyType type);
u16 Enemy_GetHeight(EnemyType type);
u16 Enemy_GetCollisionRadius(EnemyType type);

#endif /* ENEMY_H */
