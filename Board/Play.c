#include "Play.h"
#include "IERG3810_TFTLCD.h"
#include "PS2Keyboard.h"
#include "Resource.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Collision.h"
#include "GameUI.h"
#include "Stage.h"
#include "GameState.h"
#include "Explosion.h"
#include "Audio.h"
#include "th06_06.h"
#include "Background.h"
#include "Menu.h"
#include "JoyPad.h"
#include "GameOver.h"
#include <stdlib.h>

#define PS2_SCANCODE_KP0    0x70
#define PS2_SCANCODE_KP1    0x69
#define PS2_SCANCODE_KP2    0x72
#define PS2_SCANCODE_KP3    0x7A
#define PS2_SCANCODE_KP5    0x73
#define PS2_SCANCODE_KP7    0x6C
#define PS2_SCANCODE_KP8    0x75
#define PS2_SCANCODE_KP9    0x7D

#define LCD_WIDTH   240u
#define LCD_HEIGHT  320u
#define PLAYER_TRANSPARENT_COLOR 0x0000

typedef struct {
    s16 x;          // Center X position
    s16 y;          // Center Y position
    s16 prev_x;     // Previous center X for clearing
    s16 prev_y;     // Previous center Y for clearing
} Cursor;

static Cursor g_cursor;
static u8 playInitialized = 0u;
static u8 keyUpHeld;
static u8 keyDownHeld;
static u8 keyLeftHeld;
static u8 keyRightHeld;
static u8 joypadState = 0;
static u8 prevJoypadState = 0;
static u8 spriteDrawn;
static u16 shootTimer = 0u;
static const u16 SHOOT_INTERVAL = 15u;  // Shooting interval (frames)

// Player HP system
static u16 g_playerHP = 10u;
static u16 g_playerMaxHP = 10u;  /* Set according to difficulty */
static u16 invincibleTimer = 0u;  // Invincible timer (time after being hit)
static const u16 INVINCIBLE_TIME = 60u;  // 60 frames invincible time
static u16 playerHitFlashTimer = 0u;  // Player hit red flash timer
static u16 playerDeathDelayTimer = 0u;  // Player death delay timer (2 seconds)

// Score system
static u32 g_score = 0u;

static const Bitmap16 *g_playerSprite = &gBitmap_spaceShips_001;
static u16 playerHalfWidth;
static u16 playerHalfHeight;
static s16 playerMinX;
static s16 playerMaxX;
static s16 playerMinY;
static s16 playerMaxY;

static void Play_ComputeBottomLeft(s16 center_x, s16 center_y, s16 *out_x, s16 *out_y)
{//This converts center coordinates to bottom-left coordinates for drawing
 //also handles boundary checks
    s16 x = center_x - (s16)playerHalfWidth;
    s16 y = center_y - (s16)playerHalfHeight;

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    if (x > (s16)(LCD_WIDTH - g_playerSprite->width)) {
        x = (s16)(LCD_WIDTH - g_playerSprite->width);
    }
    if (y > (s16)(LCD_HEIGHT - g_playerSprite->height)) {
        y = (s16)(LCD_HEIGHT - g_playerSprite->height);
    }

    *out_x = x;
    *out_y = y;
}

static void Play_DrawPlayer(s16 center_x, s16 center_y)
{
    s16 bottom_left_x;
    s16 bottom_left_y;
    Play_ComputeBottomLeft(center_x, center_y, &bottom_left_x, &bottom_left_y);

    /* If player is hit and flashing red, use red color */
    if (playerHitFlashTimer > 0) {
        lcd_drawBitmap16_Colorize((u16)bottom_left_x,
                                  (u16)bottom_left_y,
                                  g_playerSprite,
                                  c_red,
                                  PLAYER_TRANSPARENT_COLOR);
    } else {
        lcd_drawBitmap16_ColorKey((u16)bottom_left_x,
                                  (u16)bottom_left_y,
                                  g_playerSprite,
                                  PLAYER_TRANSPARENT_COLOR);
    }
}

static void Play_ClearPlayer(s16 center_x, s16 center_y)
{
    s16 bottom_left_x;
    s16 bottom_left_y;
    Play_ComputeBottomLeft(center_x, center_y, &bottom_left_x, &bottom_left_y);

    lcd_clearBitmapArea((u16)bottom_left_x,
                        (u16)bottom_left_y,
                        g_playerSprite,
                        c_black);
}

void Play_Init(void)
{
    playerHalfWidth  = g_playerSprite->width  / 2u;
    playerHalfHeight = g_playerSprite->height / 2u;
    playerMinX = (s16)playerHalfWidth;
    playerMaxX = (s16)(LCD_WIDTH - g_playerSprite->width + playerHalfWidth);
    playerMinY = (s16)playerHalfHeight;
    playerMaxY = (s16)(LCD_HEIGHT - g_playerSprite->height + playerHalfHeight);

        // Reset player to the default spawn position (top centre by default)
        g_cursor.x = (s16)(LCD_WIDTH / 2u);
        g_cursor.y = playerMinY;
    g_cursor.prev_x = g_cursor.x;
    g_cursor.prev_y = g_cursor.y;
    playInitialized = 0u;
    keyUpHeld    = 0u;
    keyDownHeld  = 0u;
    keyLeftHeld  = 0u;
    keyRightHeld = 0u;
    spriteDrawn  = 0u;
    shootTimer   = 0u;
    
    // Initialize player HP and score (according to difficulty)
    g_playerMaxHP = Menu_GetMaxHP();
    g_playerHP = g_playerMaxHP;
    invincibleTimer = 0u;
    playerHitFlashTimer = 0u;
    playerDeathDelayTimer = 0u;
    g_score = 0u;
    
    // Initialize background system
    Background_Init();
    // Initialize enemy system
    Enemy_Init();
    // Initialize bullet system
    Bullet_Init();
    // Initialize collision system
    Collision_Init();
    // Initialize UI system
    GameUI_Init();
    // Initialize stage system  
    Stage_Init();
    // Initialize explosion system
    Explosion_Init();
    // Initialize audio system
    Audio_Init();
}



void Play_OnEnter(void)
{
    g_cursor.x = (s16)(LCD_WIDTH / 2u);
    g_cursor.y = playerMinY;
    g_cursor.prev_x = g_cursor.x;
    g_cursor.prev_y = g_cursor.y;
    playInitialized = 0u;
    keyUpHeld    = 0u;
    keyDownHeld  = 0u;
    keyLeftHeld  = 0u;
    keyRightHeld = 0u;
    spriteDrawn  = 0u;
    shootTimer   = 0u;
    
    // 重置玩家血量和分數(根據難度)
    g_playerMaxHP = Menu_GetMaxHP();
    g_playerHP = g_playerMaxHP;
    invincibleTimer = 0u;
    playerHitFlashTimer = 0u;
    playerDeathDelayTimer = 0u;
    g_score = 0u;
    
    // Reset enemy system
    Enemy_Init();
    // Reset bullet system
    Bullet_Init();
    // Reset UI system
    GameUI_Init();
    // Reset stage system 
    Stage_Reset();
    // Reset explosion system
    Explosion_Init();
    // Start playing BGM (loop)
    Audio_StartBGM(song, sizeof(song) / sizeof(song[0]), 1);
}

void Play_UpdateLogic(void)
{
    s16 new_x = g_cursor.x;
    s16 new_y = g_cursor.y;
    s16 dx = 0;
    s16 dy = 0;
    // Combine Keyboard and JoyPad input
    u8 left  = keyLeftHeld || (joypadState & JOYPAD_LEFT); //joypadState is the return value of JoyPad_Read()
    u8 right = keyRightHeld || (joypadState & JOYPAD_RIGHT);
    u8 up    = keyUpHeld || (joypadState & JOYPAD_UP);
    u8 down  = keyDownHeld || (joypadState & JOYPAD_DOWN);

    // Update player death delay timer
    if (g_playerHP == 0) {
        if (playerDeathDelayTimer > 0) {
            playerDeathDelayTimer--;
        } else {
            GameOver_SetMissionStatus(0); // 0 = Failed
            GameState_Set(STATE_GAMEOVER);
            Audio_StopBGM();
            return;
        }
        
        // If dead, skip movement and shooting, but update environment
        Audio_UpdateBGM();
        Background_Update();
        Stage_Update();
        Enemy_UpdateAll();
        Bullet_UpdateAll();
        Collision_CheckAll();
        Explosion_UpdateAll();
        return;
    }//SKIP BELOW IF DEAD

    /*Single direction*/
    if (left && !right) {
        dx = -(s16)PLAYER_MOVE_SPEED;
    } else if (right && !left) {
        dx = (s16)PLAYER_MOVE_SPEED;
    }

    if (up && !down) {
        dy = (s16)PLAYER_MOVE_SPEED;
    } else if (down && !up) {
        dy = -(s16)PLAYER_MOVE_SPEED;
    }
    /******************/
    /*Diagonal movement adjustment*/
    if ((dx != 0) && (dy != 0)) {
        s16 diagStep = (s16)((PLAYER_MOVE_SPEED * 707u + 500u) / 1000u); // approx. PLAYER_MOVE_SPEED / sqrt(2)
        if (diagStep == 0) diagStep = 1; // Ensure at least 1 pixel movement
        dx = (dx > 0) ? diagStep : (s16)(-diagStep);
        dy = (dy > 0) ? diagStep : (s16)(-diagStep); 
    }
    /*************************/

    new_x += dx;
    new_y += dy;

    if (new_x < playerMinX) new_x = playerMinX; // Boundary checks
    if (new_x > playerMaxX) new_x = playerMaxX;
    if (new_y < playerMinY) new_y = playerMinY;
    if (new_y > playerMaxY) new_y = playerMaxY;

    g_cursor.x = new_x;
    g_cursor.y = new_y;
    
    // Player auto-shooting
    if (shootTimer == 0) {
        // Fire bullet (from top center of player aircraft)
        s16 bulletX = g_cursor.x;
        s16 bulletY = g_cursor.y;

        Bullet_Spawn(BULLET_TYPE_PLAYER_NORMAL, bulletX, bulletY, 0, 5, 1);  // vy=5 Upwards
        shootTimer = SHOOT_INTERVAL;
    }
    
    // Update shooting timer
    if (shootTimer > 0) {
        shootTimer--;
    }
    
    // Update invincibility timer
    if (invincibleTimer > 0) {
        invincibleTimer--;
    }
    
    // Update player hit flash timer
    if (playerHitFlashTimer > 0) {
        playerHitFlashTimer--;
    }
    
    // Update BGM sequence
    Audio_UpdateBGM();
    // Update starry background
    Background_Update();
    // Update stage (spawn enemies based on timeline)
    Stage_Update();
    // Update enemy system
    Enemy_UpdateAll();
    // Update bullet system
    Bullet_UpdateAll();
    // Collision detection
    Collision_CheckAll();
    // Update explosion effects
    Explosion_UpdateAll();
    
    // Check if stage is complete (Boss defeated)
    if (Stage_GetState() == STAGE_STATE_COMPLETE) {
        GameOver_SetMissionStatus(1); // 1 = Completed
        GameState_Set(STATE_GAMEOVER);
        Audio_StopBGM();
    }
}

void Play_Render(void)
{
    if (!playInitialized) {
        // First render - clear screen
        lcd_fillRectangle(c_black, 0, LCD_WIDTH, 0, LCD_HEIGHT);
        playInitialized = 1u;
    }
    
    // Render starry background
    Background_Render();
    // Render enemies
    Enemy_RenderAll();
    // Render bullets
    Bullet_RenderAll();
    // Render explosion effects
    Explosion_RenderAll();
    
    // UI always renders (update score and HP)
    GameUI_SetScore(g_score);
    GameUI_SetHP(g_playerHP, g_playerMaxHP);
    GameUI_Render();
    
    // Clear previous sprite position when it changed
    if (spriteDrawn && (g_cursor.prev_x != g_cursor.x || g_cursor.prev_y != g_cursor.y)) {
        Play_ClearPlayer(g_cursor.prev_x, g_cursor.prev_y);
    }
    
    // Draw player at new position if alive
    if (g_playerHP > 0) {
        Play_DrawPlayer(g_cursor.x, g_cursor.y);
        spriteDrawn = 1u;
    } else {
        // If player is dead, ensure the last position is cleared
        if (spriteDrawn) {
            Play_ClearPlayer(g_cursor.prev_x, g_cursor.prev_y);
            spriteDrawn = 0u;
        }
    }
    
    // Update previous position
    g_cursor.prev_x = g_cursor.x;
    g_cursor.prev_y = g_cursor.y;
}

void Play_HandleEvent(const PS2KeyEvent *evt)
{
    u8 pressed = (evt->isBreak == 0u);

    switch (evt->code) {
    case PS2_SCANCODE_KP5:
        keyUpHeld = pressed;
        break;
    case PS2_SCANCODE_KP2:
        keyDownHeld = pressed;
        break;
    case PS2_SCANCODE_KP1:
        keyLeftHeld = pressed;
        break;
    case PS2_SCANCODE_KP3:
        keyRightHeld = pressed;
        break;
    
                // Test keys: spawn enemies
                case PS2_SCANCODE_KP7:
                    if (pressed) {
                        // Numpad 7: spawn TYPE_NORMAL_1
                        Enemy_Spawn(ENEMY_TYPE_NORMAL_1, 60, 330, ENEMY_MOVE_STRAIGHT_DOWN);
                    }
                    break;
                case PS2_SCANCODE_KP8:
                    if (pressed) {
                        // Numpad 8: spawn TYPE_NORMAL_2 (Zigzag movement)
                        Enemy_Spawn(ENEMY_TYPE_NORMAL_2, 120, 330, ENEMY_MOVE_ZIGZAG);
                    }
                    break;
                case PS2_SCANCODE_KP9:
                    if (pressed) {
                        // Numpad 9: spawn BOSS
                        Enemy_Spawn(ENEMY_TYPE_BOSS, 120, 350, ENEMY_MOVE_BOSS_PATTERN);
                    }
                    break;
        
    default:
        break;
    }
}

void Play_HandleJoyPad(u8 state) //called by main loop to update joypadState
{
    joypadState = state;    
    prevJoypadState = joypadState; // For edge detection if needed but not used currently
}

// ==================== Collision system interface ====================

void Play_GetPlayerPos(s16 *x, s16 *y)
{
    s16 bottomLeftX, bottomLeftY;
    Play_ComputeBottomLeft(g_cursor.x, g_cursor.y, &bottomLeftX, &bottomLeftY);
    *x = bottomLeftX;
    *y = bottomLeftY;
}

void Play_GetPlayerSize(u16 *w, u16 *h)
{
    *w = g_playerSprite->width;
    *h = g_playerSprite->height;
}

void Play_PlayerTakeDamage(u16 damage)
{
    // Invincible time - no damage
    if (invincibleTimer > 0) return;
    if (g_playerHP == 0) return; // Already dead
    
    if (g_playerHP > damage) {
        g_playerHP -= damage;
    } else {
        g_playerHP = 0;
        // Spawn player explosion effect (multiple explosions for drama)
        Explosion_Spawn(g_cursor.x, g_cursor.y);
        {
            s16 offsetX, offsetY;
            // 1st extra explosion
            offsetX = -10 + (rand() % 20);
            offsetY = -10 + (rand() % 20);
            Explosion_SpawnDelayed(g_cursor.x + offsetX, g_cursor.y + offsetY, 10);
            
            // 2nd extra explosion
            offsetX = -15 + (rand() % 30);
            offsetY = -15 + (rand() % 30);
            Explosion_SpawnDelayed(g_cursor.x + offsetX, g_cursor.y + offsetY, 25);
            
            // 3rd extra explosion
            offsetX = -20 + (rand() % 40);
            offsetY = -20 + (rand() % 40);
            Explosion_SpawnDelayed(g_cursor.x + offsetX, g_cursor.y + offsetY, 40);
            
            // 4th extra explosion
            offsetX = -10 + (rand() % 20);
            offsetY = -10 + (rand() % 20);
            Explosion_SpawnDelayed(g_cursor.x + offsetX, g_cursor.y + offsetY, 60);

            // 5th extra explosion
            offsetX = -10 + (rand() % 20);
            offsetY = -10 + (rand() % 20);
            Explosion_SpawnDelayed(g_cursor.x + offsetX, g_cursor.y + offsetY, 80);

            // 6th extra explosion
            offsetX = -15 + (rand() % 30);
            offsetY = -15 + (rand() % 30);
            Explosion_SpawnDelayed(g_cursor.x + offsetX, g_cursor.y + offsetY, 100);

            //7th extra explosion
            offsetX = -20 + (rand() % 40);
            offsetY = -20 + (rand() % 40);
            Explosion_SpawnDelayed(g_cursor.x + offsetX, g_cursor.y + offsetY, 110);

        }

        // Set 2-second delay (for explosion animation)
        playerDeathDelayTimer = 120u;  /* 120 frames = 2 seconds */
    }
    
    // Set invincible time and red flash effect
    invincibleTimer = INVINCIBLE_TIME;
    playerHitFlashTimer = 6u;  /* 6 frames red flash */
}

// ==================== Score system interface ====================

void Play_AddScore(u32 points)
{
    g_score += points;
}

u32 Play_GetScore(void)
{
    return g_score;
}

u16 Play_GetHP(void)
{
    return g_playerHP;
}
