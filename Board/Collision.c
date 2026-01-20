#include "Collision.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Play.h"

// ==================== Internal helper functions ====================

// Axis-Aligned Bounding Box (AABB) rectangle intersection test
u8 Collision_RectRect(s16 x1, s16 y1, u16 w1, u16 h1,
                      s16 x2, s16 y2, u16 w2, u16 h2)
{
    // Check if two rectangles overlap
    // Rectangle1: [x1, x1 + w1] x [y1, y1 + h1]
    // Rectangle2: [x2, x2 + w2] x [y2, y2 + h2]
    
    if (x1 + (s16)w1 < x2) return 0;  // Rectangle1 is left of Rectangle2
    if (x1 > x2 + (s16)w2) return 0;  // Rectangle1 is right of Rectangle2
    if (y1 + (s16)h1 < y2) return 0;  // Rectangle1 is below Rectangle2
    if (y1 > y2 + (s16)h2) return 0;  // Rectangle1 is above Rectangle2
    
    return 1;  // Rectangles overlap
}

// Detect collisions between player bullets and enemies
static void Collision_PlayerBulletsVsEnemies(void)
{
    u16 i, j;
    Bullet *playerBullets = Bullet_GetPlayerPool();
    Enemy *enemies = Enemy_GetPool();
    Bullet *bullet;
    Enemy *enemy;
    u16 bulletW, bulletH;
    u16 enemyW, enemyH;

    for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
        bullet = &playerBullets[i];

        if (bullet->state != BULLET_STATE_ACTIVE) continue;
        if (!bullet->sprite) continue;

        bulletW = bullet->sprite->width;
        bulletH = bullet->sprite->height;

        for (j = 0; j < MAX_ENEMIES; j++) {
            enemy = &enemies[j];

            if (!Enemy_IsActive(enemy)) continue;
            if (!enemy->sprite) continue;

            enemyW = enemy->sprite->width;
            enemyH = enemy->sprite->height;

            // Check collision
            if (Collision_RectRect(bullet->x, bullet->y, bulletW, bulletH,
                                   enemy->x, enemy->y, enemyW, enemyH)) {
                // Collision occurred
                Enemy_TakeDamage(enemy, bullet->damage);
                Bullet_Destroy(bullet);
                break; // Bullet destroyed; break out of inner loop
            }
        }
    }
}

// Detect collisions between enemy bullets and the player
static void Collision_EnemyBulletsVsPlayer(void)
{
    u16 i;
    Bullet *enemyBullets = Bullet_GetEnemyPool();
    s16 playerX, playerY;
    u16 playerW, playerH;
    Bullet *bullet;
    u16 bulletW, bulletH;

    // Get player position and size
    Play_GetPlayerPos(&playerX, &playerY);
    Play_GetPlayerSize(&playerW, &playerH);

    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        bullet = &enemyBullets[i];

        if (bullet->state != BULLET_STATE_ACTIVE) continue;
        if (!bullet->sprite) continue;

        bulletW = bullet->sprite->width;
        bulletH = bullet->sprite->height;

        // Check collision
        if (Collision_RectRect(bullet->x, bullet->y, bulletW, bulletH,
                               playerX, playerY, playerW, playerH)) {
            // Collision occurred
            Play_PlayerTakeDamage(bullet->damage);
            Bullet_Destroy(bullet);
        }
    }
}

// Detect collisions between player and enemies
static void Collision_PlayerVsEnemies(void)
{
    u16 i;
    Enemy *enemies = Enemy_GetPool();
    s16 playerX, playerY;
    u16 playerW, playerH;
    Enemy *enemy;
    u16 enemyW, enemyH;

    // Get player position and size
    Play_GetPlayerPos(&playerX, &playerY);
    Play_GetPlayerSize(&playerW, &playerH);

    for (i = 0; i < MAX_ENEMIES; i++) {
        enemy = &enemies[i];

        if (!Enemy_IsActive(enemy)) continue;
        if (!enemy->sprite) continue;

        enemyW = enemy->sprite->width;
        enemyH = enemy->sprite->height;

        // Check collision
        if (Collision_RectRect(playerX, playerY, playerW, playerH,
                               enemy->x, enemy->y, enemyW, enemyH)) {
            // Collision occurred
            // Damage the player and enemy for collision
            Play_PlayerTakeDamage(1);  // Player takes 1 damage from colliding with enemy
            Enemy_TakeDamage(enemy, 1); // Enemy takes 1 damage from collision
        }
    }
}

// ==================== Public API ====================

void Collision_Init(void)
{
    // No initialization required at the moment
}

void Collision_CheckAll(void)
{
    // Check all collision types
    Collision_PlayerBulletsVsEnemies();
    Collision_EnemyBulletsVsPlayer();
    Collision_PlayerVsEnemies();
}
