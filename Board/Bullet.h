#ifndef BULLET_H
#define BULLET_H

#include "stm32f10x.h"
#include "Resource.h"

// Bullet types
typedef enum {
    BULLET_TYPE_PLAYER_NORMAL,   // Player normal bullet
    BULLET_TYPE_ENEMY_NORMAL,    // Enemy normal bullet
    BULLET_TYPE_BOSS_MISSILE,    // Boss missile
    BULLET_TYPE_COUNT
} BulletType;

// Bullet states
typedef enum {
    BULLET_STATE_INACTIVE,
    BULLET_STATE_ACTIVE
} BulletState;

// Bullet structure
typedef struct {
    BulletState state;
    BulletType type;
    
    s16 x, y;              // Current position
    s16 prev_x, prev_y;    // Previous frame position
    s16 vx, vy;            // Velocity vector
    
    u16 damage;            // Damage value
    u8 radius;             // Collision radius
    
    const Bitmap16 *sprite; // Sprite bitmap
    
    u8 isPlayerBullet;     // Whether this is a player bullet
    u8 renderNeeded;       // Whether redraw is needed
} Bullet;

// Bullet pool sizes
#define MAX_PLAYER_BULLETS 32
#define MAX_ENEMY_BULLETS  64

// Bullet system API
void Bullet_Init(void);
void Bullet_UpdateAll(void);
void Bullet_RenderAll(void);

Bullet* Bullet_Spawn(BulletType type, s16 x, s16 y, s16 vx, s16 vy, u8 isPlayer);
void Bullet_Destroy(Bullet *bullet);

Bullet* Bullet_GetPlayerPool(void);
Bullet* Bullet_GetEnemyPool(void);
u16 Bullet_GetPlayerActiveCount(void);
u16 Bullet_GetEnemyActiveCount(void);

#endif /* BULLET_H */
