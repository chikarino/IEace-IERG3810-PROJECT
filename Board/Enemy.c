#include "Enemy.h"
#include "IERG3810_TFTLCD.h"
#include "Resource.h"
#include "Bullet.h"
#include "Play.h"
#include "Stage.h"
#include "Explosion.h"
#include <stdlib.h>

#define LCD_WIDTH   240u
#define LCD_HEIGHT  320u

// Enemy Pool
static Enemy g_enemyPool[MAX_ENEMIES];
static u16 g_activeEnemyCount = 0u;

static u16 g_bossCurrentHP = 0u;
static u16 g_bossMaxHP = 0u;
static u8  g_bossActive = 0u;

// Enemy Type Configuration Table
typedef struct {
    u16 hp; // Maximum HP
    u16 score; // Score reward
    u16 shootInterval;  // Shooting interval (frames), 0 means no shooting
    s16 baseSpeed;      // Base speed
    u16 collisionRadius; // Collision radius //NOT USED, Collision uses sprite size
    u8 canShoot; // Whether this enemy can shoot
    const Bitmap16 *sprite; // Sprite for this enemy type
} EnemyConfig;

static const EnemyConfig g_enemyConfigs[ENEMY_TYPE_COUNT] = {
    /* ENEMY_TYPE_NORMAL_1: small, fast enemy */
    /* hp, score, shootInterval, baseSpeed, collisionRadius, canShoot, sprite */
    {2, 100, 0, 2, 8, 0, &gBitmap_spaceMissiles_001},
    /* ENEMY_TYPE_NORMAL_2: medium zigzag enemy */
    {2, 200, 60, 1, 10, 1, &gBitmap_spaceShips_001},
    /* ENEMY_TYPE_BOSS: BOSS */
    {100, 5000, 50, 1, 24, 1, &gBitmap_BOSS}
};

// ==================== Internal ====================

static Enemy* Enemy_GetFreeSlot(void)
{
    u16 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (g_enemyPool[i].state == ENEMY_STATE_INACTIVE) {
            return &g_enemyPool[i];
        }
    }
    return NULL;  // Pool is full
}

// Draw enemy (using sprite)
static void Enemy_Draw(const Enemy *enemy)
{
    s16 x;
    s16 y;
    if (enemy->state != ENEMY_STATE_ACTIVE) return;
    x = enemy->x;
    y = enemy->y;
    
    /* If currently flashing red due to being hit, use red color */
    if (enemy->hitFlashTimer > 0) {
        lcd_drawBitmap16_Colorize(x, y, enemy->sprite, c_red, 0x0000);
    } else {
        /* Draw using sprite */
        lcd_drawBitmap16_ColorKey(x, y, enemy->sprite, 0x0000);
    }
}

/* Clear enemy (using precise bitmap area clearing) */
static void Enemy_Clear(const Enemy *enemy)
{
    s16 x;
    s16 y;
    x = enemy->prev_x;
    y = enemy->prev_y;
    lcd_clearBitmapArea(x, y, enemy->sprite, c_black);
}


static u8 Enemy_IsOffScreen(const Enemy *enemy)
{
    u16 margin = 100;  // Give some boundary tolerance
    
    if (enemy->x < -(s16)margin) return 1;
    if (enemy->x > (s16)(LCD_WIDTH + margin)) return 1;
    if (enemy->y < -(s16)margin) return 1;
    if (enemy->y > (s16)(LCD_HEIGHT + margin)) return 1;
    return 0;
}

// Update enemy movement (based on movement pattern)
static void Enemy_UpdateMovement(Enemy *enemy)
{
    switch (enemy->movePattern) {
    case ENEMY_MOVE_STRAIGHT_DOWN:
        enemy->y -= enemy->vy;  // Y decreases = move down
        break;
        
    case ENEMY_MOVE_ZIGZAG:
        // Zigzag movement
        enemy->y -= enemy->vy;  // Y decreases = move down
        enemy->frameCounter++;
        
        // Change direction every 30 frames
        if (enemy->frameCounter % 30 == 0) {
            enemy->vx = -enemy->vx;  // Reverse horizontal speed
        }
        enemy->x += enemy->vx;
        break;
    
    case ENEMY_MOVE_LEFT_DOWN:
        enemy->x -= enemy->vx;  // Move left
        enemy->y -= 1;  // Move down
        break;
    
    case ENEMY_MOVE_RIGHT_DOWN:
        enemy->x += enemy->vx;  // Move right
        enemy->y -= 1;  // Move down
        break;
        
    case ENEMY_MOVE_BOSS_PATTERN:
        /* BOSS movement pattern: horizontal back and forth, charge after half HP */
        enemy->frameCounter++;
        
        if (enemy->frameCounter < 80) {
            /* First 80 frames: enter from top to designated position (move down, Y decreases) */
            enemy->y -= 1;
        } else {
            /* Check if half HP */
            if (enemy->hp <= enemy->maxHp / 2) {
                /* Charge mode after half HP */
                if (enemy->moveParam1 == 0) {
                    /* moveParam1=0: Prepare to charge, fire missiles once at the top */
                    s16 leftX, rightX, missileY;
                    
                    /* Left firing point */
                    leftX = enemy->x - 8;
                    /* Right firing point */
                    rightX = enemy->x + (s16)enemy->sprite->width + 8;
                    /* Y coordinate at Boss center */
                    missileY = enemy->y + (s16)(enemy->sprite->height / 2);
                    /* Fire left missile */
                    Bullet_Spawn(BULLET_TYPE_BOSS_MISSILE, leftX, missileY, 0, 5, 0);
                    /* Fire right missile */
                    Bullet_Spawn(BULLET_TYPE_BOSS_MISSILE, rightX, missileY, 0, 5, 0);
                    
                    /* Switch to charge state */
                    enemy->moveParam1 = 1;
                } else if (enemy->moveParam1 == 1) {
                    /* moveParam1=1: Charging down */
                    enemy->y -= 3;  /* Move down 3 pixels per frame */
                    
                    /* Prepare to return when reaching bottom (y=40) */
                    if (enemy->y <= 40) {
                        enemy->moveParam1 = 2;  /* Switch to return mode */
                    }
                } else if (enemy->moveParam1 == 2) {
                    /* moveParam1=2: Returning up */
                    enemy->y += 2;  /* Move up */
                    
                    /* Return to near initial height (around y=250) to resume horizontal movement */
                    if (enemy->y >= 250) {
                        enemy->y = 250;
                        enemy->moveParam1 = 3;  /* Mark that one charge is completed */
                    }
                }
            } 
            
            /* Not half HP or charge completed: horizontal back and forth movement */
            if (enemy->hp > enemy->maxHp / 2 || enemy->moveParam1 == 3) {
                enemy->x += enemy->vx;
                
                /* Reverse direction when reaching boundaries */
                if (enemy->x <= 0 || enemy->x >= (s16)(LCD_WIDTH - enemy->sprite->width)) {
                    enemy->vx = -enemy->vx;
                }
            }
        }
        break;
        
    default:
        enemy->y += enemy->vy;
        break;
    }
}
// Update enemy shooting
static void Enemy_UpdateShooting(Enemy *enemy)
{
    s16 bulletX;
    s16 bulletY;
    
    if (!enemy->canShoot) return;
    if (enemy->shootInterval == 0) return;
    
    enemy->shootTimer++;
    
    if (enemy->shootTimer >= enemy->shootInterval) {
        enemy->shootTimer = 0;
        
        // Spawn enemy bullet (fired from the bottom center of the enemy)
        bulletX = enemy->x + (s16)(enemy->sprite->width / 2) - 2;  // Bullet width 4, centered
        bulletY = enemy->y + (s16)enemy->sprite->height;
        
        Bullet_Spawn(BULLET_TYPE_ENEMY_NORMAL, bulletX, bulletY, 0, 3, 0);
    }
}

// ==================== Public API ====================

void Enemy_Init(void)
{
    u16 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        g_enemyPool[i].state = ENEMY_STATE_INACTIVE;
        g_enemyPool[i].type = ENEMY_TYPE_NORMAL_1;
        g_enemyPool[i].x = 0;
        g_enemyPool[i].y = 0;
        g_enemyPool[i].prev_x = 0;
        g_enemyPool[i].prev_y = 0;
        g_enemyPool[i].vx = 0;
        g_enemyPool[i].vy = 0;
        g_enemyPool[i].hp = 0;
        g_enemyPool[i].frameCounter = 0;
        g_enemyPool[i].shootTimer = 0;
        g_enemyPool[i].hitFlashTimer = 0;
        g_enemyPool[i].sprite = NULL;
        g_enemyPool[i].renderNeeded = 0;
    }
    
    g_activeEnemyCount = 0u;
    g_bossCurrentHP = 0u;
    g_bossMaxHP = 0u;
    g_bossActive = 0u;
}

Enemy* Enemy_Spawn(EnemyType type, s16 x, s16 y, EnemyMovePattern pattern)
{
    Enemy *enemy;
    const EnemyConfig *cfg;
    
    if (type >= ENEMY_TYPE_COUNT) return NULL;
    
    enemy = Enemy_GetFreeSlot();
    if (enemy == NULL) return NULL;  /* Pool is full */
    
    cfg = &g_enemyConfigs[type];
    
    // Initialize enemy
    enemy->state = ENEMY_STATE_ACTIVE;
    enemy->type = type;
    enemy->movePattern = pattern;
    
    enemy->x = x;
    enemy->y = y;
    enemy->prev_x = x;
    enemy->prev_y = y;
    
    enemy->spawnX = x;
    enemy->spawnY = y;
    
    enemy->hp = cfg->hp;
    enemy->maxHp = cfg->hp;
    enemy->score = cfg->score;
    
    enemy->frameCounter = 0;
    enemy->shootTimer = 0;
    enemy->shootInterval = cfg->shootInterval;
    enemy->canShoot = cfg->canShoot;
    enemy->hitFlashTimer = 0;
    
    /* Select sprite based on enemy type */
    enemy->sprite = cfg->sprite;
        
    enemy->renderNeeded = 1;
    
    /* Configure initial velocity according to the move pattern */
    switch (pattern) {
    case ENEMY_MOVE_STRAIGHT_DOWN:
        enemy->vx = 0;
        enemy->vy = cfg->baseSpeed;  /* Subtracting Y moves the sprite downward */
        enemy->moveParam1 = 0;
        enemy->moveParam2 = 0;
        break;
        
    case ENEMY_MOVE_ZIGZAG:
        enemy->vx = cfg->baseSpeed;
        enemy->vy = cfg->baseSpeed;  /* Subtracting Y moves the sprite downward */
        enemy->moveParam1 = 0;
        enemy->moveParam2 = 0;
        break;
    
    case ENEMY_MOVE_LEFT_DOWN:
        enemy->vx = cfg->baseSpeed;
        enemy->vy = 0;
        enemy->moveParam1 = 0;
        enemy->moveParam2 = 0;
        break;
    
    case ENEMY_MOVE_RIGHT_DOWN:
        enemy->vx = cfg->baseSpeed;
        enemy->vy = 0;
        enemy->moveParam1 = 0;
        enemy->moveParam2 = 0;
        break;
        
    case ENEMY_MOVE_BOSS_PATTERN:
        enemy->vx = 1;
        enemy->vy = 0;
        enemy->moveParam1 = 0;
        enemy->moveParam2 = 0;
        break;
        
    default:
        enemy->vx = 0;
        enemy->vy = cfg->baseSpeed;  /* Subtracting Y moves the sprite downward */
        break;
    }
    
    g_activeEnemyCount++;
    if (type == ENEMY_TYPE_BOSS) {
        g_bossActive = 1u;
        g_bossMaxHP = enemy->maxHp;
        g_bossCurrentHP = enemy->hp;
    }
    return enemy;
}

void Enemy_Destroy(Enemy *enemy)
{
    if (enemy == NULL) return;
    if (enemy->state == ENEMY_STATE_INACTIVE) return;
    // Clear the enemy from the screen
    Enemy_Clear(enemy);
    enemy->state = ENEMY_STATE_INACTIVE;
    
    if (g_activeEnemyCount > 0) {
        g_activeEnemyCount--;
    }
    if (enemy->type == ENEMY_TYPE_BOSS) {
        g_bossActive = 0u;
        g_bossCurrentHP = 0u;
    }
}

void Enemy_TakeDamage(Enemy *enemy, u16 damage)
{
    const EnemyConfig *cfg;
    
    if (enemy == NULL) return;
    if (enemy->state != ENEMY_STATE_ACTIVE) return;
    
    if (enemy->hp > damage) {
        enemy->hp -= damage;
        if (enemy->type == ENEMY_TYPE_BOSS) {
            g_bossCurrentHP = enemy->hp;
        }
        /* Enable 3-frame red flash when damaged */
        enemy->hitFlashTimer = 3u;
        enemy->renderNeeded = 1u;
    } else {
        enemy->hp = 0;
        enemy->state = ENEMY_STATE_DYING;
        if (enemy->type == ENEMY_TYPE_BOSS) {
            g_bossCurrentHP = 0u;
            g_bossActive = 0u;
        }
        
        // Spawn explosion at the enemy center
        Explosion_Spawn(enemy->x, enemy->y);
        
        // If the boss dies, trigger multiple delayed explosions for drama
        if (enemy->type == ENEMY_TYPE_BOSS) {
            s16 offsetX, offsetY;
            /* Create extra explosions around the boss */
            offsetX = -15 + (rand() % 10);
            offsetY = -15 + (rand() % 10);
            Explosion_SpawnDelayed(enemy->x + offsetX, enemy->y + offsetY, 15);
            
            offsetX = 15 + (rand() % 10);
            offsetY = -15 + (rand() % 10);
            Explosion_SpawnDelayed(enemy->x + offsetX, enemy->y + offsetY, 30);
            
            offsetX = -15 + (rand() % 10);
            offsetY = 15 + (rand() % 10);
            Explosion_SpawnDelayed(enemy->x + offsetX, enemy->y + offsetY, 45);
            
            offsetX = 15 + (rand() % 10);
            offsetY = 15 + (rand() % 10);
            Explosion_SpawnDelayed(enemy->x + offsetX, enemy->y + offsetY, 60);

            // 5th extra explosion
            offsetX = -10 + (rand() % 20);
            offsetY = -10 + (rand() % 20);
            Explosion_SpawnDelayed(enemy->x + offsetX, enemy->y + offsetY, 80);

            // 6th extra explosion
            offsetX = -15 + (rand() % 30);
            offsetY = -15 + (rand() % 30);
            Explosion_SpawnDelayed(enemy->x + offsetX, enemy->y + offsetY, 100);

            //7th extra explosion
            offsetX = -20 + (rand() % 40);
            offsetY = -20 + (rand() % 40);
            Explosion_SpawnDelayed(enemy->x + offsetX, enemy->y + offsetY, 110);
        }
        
        // Read enemy config and award score
        cfg = &g_enemyConfigs[enemy->type];
        Play_AddScore(cfg->score);
        
        // Notify the stage system if the boss is defeated
        if (enemy->type == ENEMY_TYPE_BOSS) {
            Stage_OnBossDefeated();
        }
        
        // Destroy immediately (Explosion system handles animation)
        Enemy_Destroy(enemy);
    }
}

u8 Enemy_GetBossHP(u16 *current, u16 *max)
{
    if (current != NULL) {
        *current = g_bossCurrentHP;
    }
    if (max != NULL) {
        *max = g_bossMaxHP;
    }
    return g_bossActive;
}

void Enemy_UpdateAll(void)
{
    u16 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        Enemy *enemy = &g_enemyPool[i];
        
        if (enemy->state != ENEMY_STATE_ACTIVE) continue;
        
        /* Update movement */
        Enemy_UpdateMovement(enemy);
        
        /* Update shooting */
        Enemy_UpdateShooting(enemy);
        
        /* Update hit-flash timer */
        if (enemy->hitFlashTimer > 0) {
            enemy->hitFlashTimer--;
            enemy->renderNeeded = 1u;
        }
        
        /* Check if the enemy left the screen */
        if (Enemy_IsOffScreen(enemy)) {
            Enemy_Destroy(enemy);
            continue;
        }
        
        /* Flag for redraw (active enemies need repaint every frame) */
        enemy->renderNeeded = 1;
    }
}

void Enemy_RenderAll(void)
{
    u16 i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        Enemy *enemy = &g_enemyPool[i];
        
        if (enemy->state != ENEMY_STATE_ACTIVE) continue;
        if (!enemy->renderNeeded) continue;
        
        /* Clear the previous position when movement occurred */
        if (enemy->x != enemy->prev_x || enemy->y != enemy->prev_y) {
            Enemy_Clear(enemy);
        }
        
        /* Draw at the new position */
        Enemy_Draw(enemy);
        
        /* Cache the position for the next frame */
        enemy->prev_x = enemy->x;
        enemy->prev_y = enemy->y;
        enemy->renderNeeded = 0;
    }
}

u8 Enemy_IsActive(const Enemy *enemy)
{
    if (enemy == NULL) return 0;
    return enemy->state == ENEMY_STATE_ACTIVE;
}

Enemy* Enemy_GetPool(void)
{
    return g_enemyPool;
}

u16 Enemy_GetActiveCount(void)
{
    return g_activeEnemyCount;
}

const Enemy* Enemy_Get(u8 index)
{
    if (index >= MAX_ENEMIES) return NULL;
    return &g_enemyPool[index];
}

u16 Enemy_GetWidth(EnemyType type)
{
    if (type >= ENEMY_TYPE_COUNT) return 0;
    if (g_enemyConfigs[type].sprite == NULL) return 0;
    return g_enemyConfigs[type].sprite->width;
}

u16 Enemy_GetHeight(EnemyType type)
{
    if (type >= ENEMY_TYPE_COUNT) return 0;
    if (g_enemyConfigs[type].sprite == NULL) return 0;
    return g_enemyConfigs[type].sprite->height;
}

u16 Enemy_GetCollisionRadius(EnemyType type)
{
    if (type >= ENEMY_TYPE_COUNT) return 0;
    return g_enemyConfigs[type].collisionRadius;
}
