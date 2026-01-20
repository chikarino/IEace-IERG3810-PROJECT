#include "Bullet.h"
#include "IERG3810_TFTLCD.h"
#include "Resource.h"
#include <stdlib.h>

#define LCD_WIDTH   240u
#define LCD_HEIGHT  320u

// Bullet object pools
static Bullet g_playerBulletPool[MAX_PLAYER_BULLETS];
static Bullet g_enemyBulletPool[MAX_ENEMY_BULLETS];
static u16 g_playerBulletCount = 0u;
static u16 g_enemyBulletCount = 0u;

// Bullet type configuration table
typedef struct {
    u16 damage;
    const Bitmap16 *sprite;
} BulletConfig;

static const BulletConfig g_bulletConfigs[BULLET_TYPE_COUNT] = { //Pairs with BulletType enum in Bullet.h
    /* BULLET_TYPE_PLAYER_NORMAL: Player normal bullet */
    /* damage, sprite */
    {1, &gBitmap_BulletDOWN},
    
    /* BULLET_TYPE_ENEMY_NORMAL: Enemy normal bullet */
    {1, &gBitmap_BulletDOWN},
    
    /* BULLET_TYPE_BOSS_MISSILE: Boss missile */
    {2, &gBitmap_spaceMissiles_001}
};

// ==================== Internal helper functions ====================

// Find an inactive slot from the player bullet pool
static Bullet* Bullet_GetFreePlayerSlot(void)
{
    u16 i;
    for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (g_playerBulletPool[i].state == BULLET_STATE_INACTIVE) {
            return &g_playerBulletPool[i];
        }
    }
    return NULL;
}

// Find an inactive slot from the enemy bullet pool
static Bullet* Bullet_GetFreeEnemySlot(void)
{
    u16 i;
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (g_enemyBulletPool[i].state == BULLET_STATE_INACTIVE) {
            return &g_enemyBulletPool[i];
        }
    }
    return NULL;
}

// Draw a bullet using its sprite
static void Bullet_Draw(const Bullet *bullet)
{
    s16 x;
    s16 y;
    if (bullet->state != BULLET_STATE_ACTIVE) return;
    x = bullet->x;
    y = bullet->y;
    lcd_drawBitmap16_ColorKey(x, y, bullet->sprite, 0x0000);
}
static void Bullet_Clear(const Bullet *bullet)
{
    s16 x;
    s16 y;
    x = bullet->prev_x;
    y = bullet->prev_y;
    lcd_clearBitmapArea(x, y, bullet->sprite, c_black);
}

// Check if bullet is off-screen
static u8 Bullet_IsOffScreen(const Bullet *bullet)
{
    if (bullet->x < -10) return 1;
    if (bullet->x > (s16)(LCD_WIDTH + 10)) return 1;
    if (bullet->y < -10) return 1;
    if (bullet->y > (s16)(LCD_HEIGHT + 10)) return 1;
    return 0;
}

// ==================== Public API implementations ====================

void Bullet_Init(void)
{
    u16 i;
    
    // Initialize player bullet pool
    for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
        g_playerBulletPool[i].state = BULLET_STATE_INACTIVE;
        g_playerBulletPool[i].x = 0;
        g_playerBulletPool[i].y = 0;
        g_playerBulletPool[i].prev_x = 0;
        g_playerBulletPool[i].prev_y = 0;
        g_playerBulletPool[i].renderNeeded = 0;
    }
    
    // Initialize enemy bullet pool
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        g_enemyBulletPool[i].state = BULLET_STATE_INACTIVE;
        g_enemyBulletPool[i].x = 0;
        g_enemyBulletPool[i].y = 0;
        g_enemyBulletPool[i].prev_x = 0;
        g_enemyBulletPool[i].prev_y = 0;
        g_enemyBulletPool[i].renderNeeded = 0;
    }
    
    g_playerBulletCount = 0u;
    g_enemyBulletCount = 0u;
}

Bullet* Bullet_Spawn(BulletType type, s16 x, s16 y, s16 vx, s16 vy, u8 isPlayer)
{
    //Note: type must be valid BulletType enum value
    Bullet *bullet;
    const BulletConfig *cfg;
    
    if (type >= BULLET_TYPE_COUNT) return NULL;
    
    // Choose the appropriate pool based on bullet ownership
    if (isPlayer) {
        bullet = Bullet_GetFreePlayerSlot();
        if (bullet == NULL) return NULL;  // Pool full
        g_playerBulletCount++;
    } else {
        bullet = Bullet_GetFreeEnemySlot();
        if (bullet == NULL) return NULL;  // Pool full
        g_enemyBulletCount++;
    }
    
    cfg = &g_bulletConfigs[type];
    
    // Initialize bullet properties
    bullet->state = BULLET_STATE_ACTIVE;
    bullet->type = type;
    bullet->isPlayerBullet = isPlayer;
    
    bullet->x = x;
    bullet->y = y;
    bullet->prev_x = x;
    bullet->prev_y = y;
    
    bullet->vx = vx;
    bullet->vy = vy;
    
    bullet->damage = cfg->damage;
    bullet->sprite = cfg->sprite;
    
    bullet->renderNeeded = 1;
    
    return bullet;
}

void Bullet_Destroy(Bullet *bullet)
{
    if (bullet == NULL) return;
    if (bullet->state == BULLET_STATE_INACTIVE) return;
    
    // Clear bullet graphic from screen
    Bullet_Clear(bullet);
    
    bullet->state = BULLET_STATE_INACTIVE;
    
    // Update active bullet counts
    if (bullet->isPlayerBullet) {
        if (g_playerBulletCount > 0) g_playerBulletCount--;
    } else {
        if (g_enemyBulletCount > 0) g_enemyBulletCount--;
    }
}

void Bullet_UpdateAll(void)
{
    u16 i;
    
    // Update player bullets
    for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
        Bullet *bullet = &g_playerBulletPool[i];
        
        if (bullet->state != BULLET_STATE_ACTIVE) continue;
        
        // Update position
        bullet->x += bullet->vx;
        bullet->y += bullet->vy;  // Player bullets move up (Y increases)
        
        // Check if off-screen
        if (Bullet_IsOffScreen(bullet)) {
            Bullet_Destroy(bullet);
            continue;
        }
        
        // Mark for redraw
        bullet->renderNeeded = 1;
    }
    
    // Update enemy bullets
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        Bullet *bullet = &g_enemyBulletPool[i];
        
        if (bullet->state != BULLET_STATE_ACTIVE) continue;
        
        // Update position
        bullet->x += bullet->vx;
        bullet->y -= bullet->vy;  // Enemy bullets move down (Y decreases)
        
        // Check if off-screen
        if (Bullet_IsOffScreen(bullet)) {
            Bullet_Destroy(bullet);
            continue;
        }
        
        // Mark for redraw
        bullet->renderNeeded = 1;
    }
}

void Bullet_RenderAll(void)
{
    u16 i;
    
    // Render player bullets
    for (i = 0; i < MAX_PLAYER_BULLETS; i++) {
        Bullet *bullet = &g_playerBulletPool[i];
        
        if (bullet->state != BULLET_STATE_ACTIVE) continue;
        if (!bullet->renderNeeded) continue;
        
        // Clear old sprite area if position changed
        if (bullet->x != bullet->prev_x || bullet->y != bullet->prev_y) {
            Bullet_Clear(bullet);
        }
        
        // Draw at the new position
        Bullet_Draw(bullet);
        
        // Update previous position for the next frame
        bullet->prev_x = bullet->x;
        bullet->prev_y = bullet->y;
        
        bullet->renderNeeded = 0;
    }
    
    // Render enemy bullets
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        Bullet *bullet = &g_enemyBulletPool[i];
        
        if (bullet->state != BULLET_STATE_ACTIVE) continue;
        if (!bullet->renderNeeded) continue;
        
        // Clear old sprite area if position changed
        if (bullet->x != bullet->prev_x || bullet->y != bullet->prev_y) {
            Bullet_Clear(bullet);
        }
        
        // Draw at the new position
        Bullet_Draw(bullet);
        
        // Update previous position for the next frame
        bullet->prev_x = bullet->x;
        bullet->prev_y = bullet->y;
        
        bullet->renderNeeded = 0;
    }
}

Bullet* Bullet_GetPlayerPool(void)
{
    return g_playerBulletPool;
}

Bullet* Bullet_GetEnemyPool(void)
{
    return g_enemyBulletPool;
}

u16 Bullet_GetPlayerActiveCount(void)
{
    return g_playerBulletCount;
}

u16 Bullet_GetEnemyActiveCount(void)
{
    return g_enemyBulletCount;
}
