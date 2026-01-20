// Menu.c - main menu rendering and PS/2 keypad navigation helpers
#include "Menu.h"
#include "IERG3810_TFTLCD.h"
#include "resource.h"
#include "Delay.h"
#include "JoyPad.h"

#define PS2_SCANCODE_KP0    0x70
#define PS2_SCANCODE_KP1    0x69
#define PS2_SCANCODE_KP2    0x72
#define PS2_SCANCODE_KP3    0x7A
#define PS2_SCANCODE_KP5    0x73
#define PS2_SCANCODE_ENTER  0x5A

#define MENU_MAIN_OPTION_COUNT       3u
#define MENU_DIFFICULTY_OPTION_COUNT 2u



#define flightInterval       30u  /* 30 * 33ms ≈ 1 秒 */
#define flightSpeed        6     /* 每次更新移動 6 pixels */
#define flightStart    (-12)   /* 隱藏在畫面左外 */
#define flightEnd              240    /* 飛到這裡就停止 */
#define flyingHeight           245    /* 標題下方位置 */

#define cannotFly        0u
#define flying      1u
#define arrivedToBeRemoved       2u // 狀態：箭頭飛出畫面後清除



static const u16 kMainOptionY[MENU_MAIN_OPTION_COUNT] = {180u, 160u, 140u};
static const char *const kMainOptionLabels[MENU_MAIN_OPTION_COUNT] = {
    "Play",
    "How to Play",
    "Set Difficulty"
};

static const u16 kDifficultyOptionY[MENU_DIFFICULTY_OPTION_COUNT] = {260u, 240u};
static const char *const kDifficultyLabels[MENU_DIFFICULTY_OPTION_COUNT] = {
    "Normal",
    "Hard"
};

typedef enum {
    MENU_MAIN,
    MENU_HOW_TO_PLAY,
    MENU_DIFFICULTY
} MenuState;

// Persistent menu state shared across render and input cycles.
static MenuState currentMenuState;
static u8 selectedDifficulty;       // difficulty index exposed via Menu_GetDifficulty
static u8 mainMenuSelection;        // cursor for MAIN screen options
static u8 difficultyMenuSelection;  // cursor for DIFFICULTY screen options
static u8 menuDirty;                // forces next Menu_Render to redraw LCD
static u8 renderStateInitialized;   // tracks whether we have drawn current state once, 0 = no, 1 = yes
static MenuState lastRenderState;   // cached state used to detect transitions
static u8 lastMainSelection;        // last painted selection for MAIN menu
static u8 lastDifficultySelection;  // last painted selection for DIFFICULTY menu
static u16 arrowTicker;             // 10ms tick accumulator for arrow timing
static s16 arrowX;                  // current arrow x-position (may be off-screen while negative)
static u8 arrowState;               // cannotFly/flying/arrivedToBeRemoved

// Draw a single menu line and mark it active when the selected flag is non-zero.
static void Menu_DrawOptionLine(u16 x, u16 y, const char *label, int selected)
{
    lcd_showChar(x, y, selected ? '>' : ' ', c_black, c_white);
    lcd_showString(x + 12, y, label, c_black, c_white);
}

static void Menu_DrawArrowIfVisible(void)
{
    if (arrowState == cannotFly) return;
    if (arrowX < 0 || arrowX > flightEnd) return;
    lcd_showChar((u16)arrowX, flyingHeight, '>', c_black, c_white);
}

// Render the main menu screen; invoked by Menu_Render when state == MENU_MAIN.
static void Menu_DrawMain(void)
{
    u8 i;
    //const char *game_name     = "Thunder Fighter!";
    const char *cuid_line1    = "1155208116 XIAO KA MONG";
    const char *cuid_line2    = "1155203192 NG KWAN HO";
    const char *footer_hint_1 = "5/1/2/3 or D-pad to move";
    const char *footer_hint_2 = "Press Enter/A to select";

    lcd_fillRectangle(c_white, 0, 240, 0, 320);
    //lcd_showString(20, 260, game_name,  c_black, c_white);
    lcd_drawBitmap16_ColorKey(10, 279, &gBitmap_Title, c_black);
    lcd_drawBitmap16_ColorKey(20, 210, &gBitmap_spaceMissiles_001, c_black);
    lcd_showString(20,  40, cuid_line1, c_black, c_white);
    lcd_showString(20,  60, cuid_line2, c_black, c_white);


    for (i = 0; i < MENU_MAIN_OPTION_COUNT; ++i) {
        Menu_DrawOptionLine(20, kMainOptionY[i], kMainOptionLabels[i], mainMenuSelection == i);
    }

    lcd_showString(20, 120, "Difficulty:", c_black, c_white);
    if(selectedDifficulty==0) lcd_showString(116, 120, "Normal", c_green, c_white);
    else if(selectedDifficulty==1) lcd_showString(116, 120, "Hard", c_red, c_white);
    lcd_showString(20, 100, footer_hint_1, c_black, c_white);
    lcd_showString(20,  80, footer_hint_2, c_black, c_white);

    Menu_DrawArrowIfVisible(); //draw arrow if in flying state
}

// Render the control guide screen.
static void Menu_DrawHowToPlay(void)
{
    const char *instruction1 = "5 = Up, 2 = Down";
    const char *instruction2 = "1 = Left, 3 = Right";
    const char *instruction3 = "Use D-pad for joypad";
    const char *instruction4 = "Shoot enemies, dodge";
    const char *instruction5 = "Press 1/B to go back";

    lcd_fillRectangle(c_white, 0, 240, 0, 320);
    lcd_showString(20, 260, instruction1, c_black, c_white);
    lcd_showString(20, 240, instruction2, c_black, c_white);
    lcd_showString(20, 220, instruction3, c_black, c_white);    
    lcd_showString(20, 200, instruction4, c_black, c_white);
    lcd_showString(20, 180, "Your target: ", c_black, c_white);
    lcd_drawBitmap16_ColorKey(20, 180-40, &gBitmap_BOSS, c_black);
    lcd_showString(20, 180-60, "Boss's guards: ", c_black, c_white);
    lcd_drawBitmap16_ColorKey(20, 180-40-42-20, &gBitmap_spaceMissiles_001, c_black);
    lcd_drawBitmap16_ColorKey(20+20, 180-40-20-20, &gBitmap_spaceShips_001, c_black);
    lcd_showString(20, 30, instruction5, c_black, c_white);
}

// Render the difficulty picker screen.
static void Menu_DrawDifficulty(void)
{
    u8 i;
    const char *hintline0  = "Normal => 9HP, Hard => 5HP";
    const char *hintLine2   = "Enter/A to confirm";
    const char *hintLine3   = "1/B to cancel";

    lcd_fillRectangle(c_white, 0, 240, 0, 320);

    for (i = 0; i < MENU_DIFFICULTY_OPTION_COUNT; ++i) {
        Menu_DrawOptionLine(20, kDifficultyOptionY[i], kDifficultyLabels[i], difficultyMenuSelection == i);
    }
    lcd_showString(20, 280, hintline0, c_black, c_white);
    lcd_showString(20, 180, hintLine2, c_black, c_white);
    lcd_showString(20, 160, hintLine3, c_black, c_white);
}

// Rewrite only the menu rows affected by selection changes to avoid full-screen redraw.
static void Menu_UpdateMainCursor(u8 previous, u8 current)
{
    if (previous < MENU_MAIN_OPTION_COUNT) {
        Menu_DrawOptionLine(20, kMainOptionY[previous], kMainOptionLabels[previous], 0);
    }
    if (current < MENU_MAIN_OPTION_COUNT) {
        Menu_DrawOptionLine(20, kMainOptionY[current], kMainOptionLabels[current], 1);
    }
}

static void Menu_UpdateDifficultyCursor(u8 previous, u8 current)
{
    if (previous < MENU_DIFFICULTY_OPTION_COUNT) {
        Menu_DrawOptionLine(20, kDifficultyOptionY[previous], kDifficultyLabels[previous], 0);
    }
    if (current < MENU_DIFFICULTY_OPTION_COUNT) {
        Menu_DrawOptionLine(20, kDifficultyOptionY[current], kDifficultyLabels[current], 1);
    }
}
// Menu_Init is called from App_Init (miniProjectMain.c) before the scheduler runs.
void Menu_Init(void)
{
    currentMenuState   = MENU_MAIN;
    selectedDifficulty = 0;
    mainMenuSelection  = 0;
    difficultyMenuSelection = 0;
    menuDirty          = 1;
    renderStateInitialized = 0;
    lastRenderState        = MENU_MAIN;
    lastMainSelection      = 0;
    lastDifficultySelection = 0;
    arrowTicker            = 0;
    arrowX                 = flightStart;
    arrowState             = cannotFly;
}

// Menu_OnEnter runs whenever GameState switches to STATE_MENU (GameState_Set).
void Menu_OnEnter(void)
{
    currentMenuState = MENU_MAIN;
    mainMenuSelection = 0;
    menuDirty        = 1;
    renderStateInitialized = 0;
    lastRenderState        = MENU_MAIN;
    lastMainSelection      = mainMenuSelection;
    lastDifficultySelection = difficultyMenuSelection;
    arrowTicker            = 0;
    arrowX                 = flightStart;
    arrowState             = cannotFly;
}

// Menu_UpdateLogic runs from GameState_Update; hook non-render timers/animations here.
void Menu_UpdateLogic(void) //animation moved to Menu_Animation and called in Menu_Render
{
    // s16 nextX;
    // if (currentMenuState != MENU_MAIN) { //if not in main menu, do not show arrow
    //     if (arrowState != cannotFly && arrowX >= 0 && arrowX <= flightEnd) {
    //         lcd_showChar((u16)arrowX, flyingHeight, ' ', c_black, c_white);
    //     }
    //     arrowState  = cannotFly;
    //     arrowTicker = 0;
    //     arrowX      = flightStart;
    //     return;
    // }

    // if (arrowState == cannotFly) { //accumulate ticks until it's time to launch the arrow
    //     if (arrowTicker < flightInterval) {
    //         arrowTicker++;
    //     } else { // time to launch the arrow
    //         arrowTicker = 0;
    //         arrowState  = flying;
    //         arrowX      = flightStart;
    //     }
    //     return;
    // }

    // if (arrowState == flying) {
    //     s16 prevX = arrowX;
    //     nextX = arrowX + flightSpeed;

    //     if (prevX >= 0 && prevX <= flightEnd) {
    //         lcd_showChar((u16)prevX, flyingHeight, ' ', c_black, c_white);
    //     }

    //     if (nextX >= flightEnd) {
    //         arrowX = flightEnd;
    //         lcd_showChar((u16)arrowX, flyingHeight, '>', c_black, c_white);
    //         arrowState = arrivedToBeRemoved;
    //         return;
    //     }

    //     arrowX = nextX;
    //     if (arrowX >= 0 && arrowX <= flightEnd) {
    //         lcd_showChar((u16)arrowX, flyingHeight, '>', c_black, c_white);
    //     }
    //     return;
    // }

    // if (arrowState == arrivedToBeRemoved) {
    //     if (arrowX >= 0 && arrowX <= flightEnd) {
    //         lcd_showChar((u16)arrowX, flyingHeight, ' ', c_black, c_white);
    //     }
    //     arrowState  = cannotFly;
    //     arrowTicker = 0;
    //     arrowX      = flightStart;
    // }
}

void Menu_Animation(void){
    s16 nextX;
    if (currentMenuState != MENU_MAIN) { //if not in main menu, do not show arrow
        if (arrowState != cannotFly && arrowX >= 0 && arrowX <= flightEnd) {
            //lcd_showChar((u16)arrowX, flyingHeight, ' ', c_black, c_white);
            lcd_drawBitmap16_ColorKey(arrowX, flyingHeight, &gBitmap_spaceShips_001_faceRight, c_black);
        }
        arrowState  = cannotFly;
        arrowTicker = 0;
        arrowX      = flightStart;
        return;
    }

    if (arrowState == cannotFly) { //accumulate ticks until it's time to launch the arrow
        if (arrowTicker < flightInterval) {
            arrowTicker++;
        } else { // time to launch the arrow
            arrowTicker = 0;
            arrowState  = flying;
            arrowX      = flightStart;
        }
        return;
    }

    if (arrowState == flying) {
        s16 prevX = arrowX;
        nextX = arrowX + flightSpeed;

        if (prevX >= 0 && prevX <= flightEnd) {
            //lcd_showChar((u16)prevX, flyingHeight, ' ', c_black, c_white);
            lcd_clearBitmapArea(prevX, flyingHeight, &gBitmap_spaceShips_001_faceRight, c_white);
        }

        if (nextX >= flightEnd) {
            arrowX = flightEnd;
            //lcd_showChar((u16)arrowX, flyingHeight, '>', c_black, c_white);
            lcd_drawBitmap16_ColorKey(arrowX, flyingHeight, &gBitmap_spaceShips_001_faceRight, c_black);
            arrowState = arrivedToBeRemoved;
            return;
        }

        arrowX = nextX;
        if (arrowX >= 0 && arrowX <= flightEnd) {
            //lcd_showChar((u16)arrowX, flyingHeight, '>', c_black, c_white);
            lcd_drawBitmap16_ColorKey(arrowX, flyingHeight, &gBitmap_spaceShips_001_faceRight, c_black);
        }
        return;
    }

    if (arrowState == arrivedToBeRemoved) {
        if (arrowX >= 0 && arrowX <= flightEnd) {
            //lcd_showChar((u16)arrowX, flyingHeight, ' ', c_black, c_white);
            lcd_clearBitmapArea(arrowX, flyingHeight, &gBitmap_spaceShips_001_faceRight, c_white);
        }
        arrowState  = cannotFly;
        arrowTicker = 0;
        arrowX      = flightStart;
    }
}


// Menu_Render is called via GameState_Render while STATE_MENU is active.
void Menu_Render(void)
{
    Menu_Animation();
    if (!menuDirty && renderStateInitialized) return;

    if (!renderStateInitialized || lastRenderState != currentMenuState) { //full redraw on state change
        switch (currentMenuState) {
        case MENU_MAIN:
            Menu_DrawMain();
            break;
        case MENU_HOW_TO_PLAY:
            Menu_DrawHowToPlay();
            break;
        case MENU_DIFFICULTY:
            Menu_DrawDifficulty();
            break;
        default:
            break;
        }

        renderStateInitialized = 1;
        lastRenderState        = currentMenuState;
        lastMainSelection      = mainMenuSelection;
        lastDifficultySelection = difficultyMenuSelection;
        menuDirty = 0;
        return;
    }

    switch (currentMenuState) {
    case MENU_MAIN:
        if (mainMenuSelection != lastMainSelection) {
            Menu_UpdateMainCursor(lastMainSelection, mainMenuSelection);
            lastMainSelection = mainMenuSelection;
        }
        break;
    case MENU_DIFFICULTY:
        if (difficultyMenuSelection != lastDifficultySelection) {
            Menu_UpdateDifficultyCursor(lastDifficultySelection, difficultyMenuSelection);
            lastDifficultySelection = difficultyMenuSelection;
        }
        break;
    case MENU_HOW_TO_PLAY:
    default:
        break;
    }

    menuDirty = 0;
}

// Menu_HandleEvent is consumed by Task_Input (miniProjectMain.c) to map keypad events to menu actions.
MenuCommand Menu_HandleEvent(const PS2KeyEvent *evt)//MenuCommand is a typedef enum
{
    if (evt->isBreak) return MENU_CMD_NONE;

    switch (currentMenuState) {
    case MENU_MAIN:
        // Numpad: 5 = up, 2 = down, Enter = confirm; 1/3 currently unused on this screen.
        if (evt->code == PS2_SCANCODE_KP5) { /* up */
            if (mainMenuSelection > 0) {
                mainMenuSelection--;
                menuDirty = 1;
            }
        } else if (evt->code == PS2_SCANCODE_KP2) { /* down */
            if (mainMenuSelection + 1u < MENU_MAIN_OPTION_COUNT) {
                mainMenuSelection++;
                menuDirty = 1;
            }
        } else if (evt->code == PS2_SCANCODE_KP1) { /* left/back - no action in main menu */
            /* intentionally empty */
        } else if (evt->code == PS2_SCANCODE_KP3) { /* right - no action in main menu */
            /* intentionally empty */
        } else if (evt->code == PS2_SCANCODE_ENTER) {
            if (mainMenuSelection == 0u) {
                //return MENU_CMD_START_GAME changes gamestate to STATE_PLAY!
                return MENU_CMD_START_GAME;
            } else if (mainMenuSelection == 1u) {
                currentMenuState = MENU_HOW_TO_PLAY;
                menuDirty        = 1;
                renderStateInitialized = 0;
                lastRenderState        = MENU_HOW_TO_PLAY;
            } else if (mainMenuSelection == 2u) {
                currentMenuState        = MENU_DIFFICULTY;
                difficultyMenuSelection = selectedDifficulty;
                menuDirty               = 1;
                renderStateInitialized  = 0;
                lastRenderState         = MENU_DIFFICULTY;
            }
        }
        break;

    case MENU_HOW_TO_PLAY:
        // Allow Enter or keypad 1 to return to the main screen.
        if (evt->code == PS2_SCANCODE_KP1 || evt->code == PS2_SCANCODE_ENTER) {
            currentMenuState = MENU_MAIN;
            menuDirty        = 1;
            renderStateInitialized = 0;
            lastRenderState        = MENU_MAIN;
        }
        break;

    case MENU_DIFFICULTY:
        // Reuse 5/2 to move through difficulty list, Enter confirms, 1 cancels.
        if (evt->code == PS2_SCANCODE_KP5) {
            if (difficultyMenuSelection > 0) {
                difficultyMenuSelection--;
                menuDirty = 1;
            }
        } else if (evt->code == PS2_SCANCODE_KP2) {
            if (difficultyMenuSelection + 1u < MENU_DIFFICULTY_OPTION_COUNT) {
                difficultyMenuSelection++;
                menuDirty = 1;
            }
        } else if (evt->code == PS2_SCANCODE_KP1) {
            currentMenuState = MENU_MAIN;
            menuDirty        = 1;
            renderStateInitialized = 0;
            lastRenderState        = MENU_MAIN;
        } else if (evt->code == PS2_SCANCODE_ENTER) {
            selectedDifficulty = difficultyMenuSelection;
            currentMenuState   = MENU_MAIN;
            menuDirty          = 1;
            renderStateInitialized = 0;
            lastRenderState        = MENU_MAIN;
        }
        break;
    default:
        break;
    }

    return MENU_CMD_NONE;
}

MenuCommand Menu_HandleJoyPad(u8 joypadState) //MenuCommand is a typedef enum
{
    static u8 lastJoypadState = 0; //static variable to hold the last state across function calls
    u8 changed = joypadState ^ lastJoypadState;//XOR to find changed bits
    u8 pressed = changed & joypadState;//edge detection for newly pressed buttons
    lastJoypadState = joypadState;//update last state

    if (!pressed) return MENU_CMD_NONE;

    switch (currentMenuState) {
    case MENU_MAIN:
        if (pressed & JOYPAD_UP) {
            if (mainMenuSelection > 0) {
                mainMenuSelection--;
                menuDirty = 1;
            }
        } else if (pressed & JOYPAD_DOWN) {
            if (mainMenuSelection + 1u < MENU_MAIN_OPTION_COUNT) {
                mainMenuSelection++;
                menuDirty = 1;
            }
        } else if (pressed & (JOYPAD_A)) {
            if (mainMenuSelection == 0u) {
                return MENU_CMD_START_GAME;
            } else if (mainMenuSelection == 1u) {
                currentMenuState = MENU_HOW_TO_PLAY;
                menuDirty        = 1;
                renderStateInitialized = 0;
                lastRenderState        = MENU_HOW_TO_PLAY;
            } else if (mainMenuSelection == 2u) {
                currentMenuState        = MENU_DIFFICULTY;
                difficultyMenuSelection = selectedDifficulty;
                menuDirty               = 1;
                renderStateInitialized  = 0;
                lastRenderState         = MENU_DIFFICULTY;
            }
        }
        break;

    case MENU_HOW_TO_PLAY:
        if (pressed & (JOYPAD_B)) {
            currentMenuState = MENU_MAIN;
            menuDirty        = 1;
            renderStateInitialized = 0;
            lastRenderState        = MENU_MAIN;
        }
        break;

    case MENU_DIFFICULTY:
        if (pressed & JOYPAD_UP) {
            if (difficultyMenuSelection > 0) {
                difficultyMenuSelection--;
                menuDirty = 1;
            }
        } else if (pressed & JOYPAD_DOWN) {
            if (difficultyMenuSelection + 1u < MENU_DIFFICULTY_OPTION_COUNT) {
                difficultyMenuSelection++;
                menuDirty = 1;
            }
        } else if (pressed & JOYPAD_B) {
            currentMenuState = MENU_MAIN;
            menuDirty        = 1;
            renderStateInitialized = 0;
            lastRenderState        = MENU_MAIN;
        } else if (pressed & (JOYPAD_A | JOYPAD_START)) {
            selectedDifficulty = difficultyMenuSelection;
            currentMenuState   = MENU_MAIN;
            menuDirty          = 1;
            renderStateInitialized = 0;
            lastRenderState        = MENU_MAIN;
        }
        break;
    default:
        break;
    }

    return MENU_CMD_NONE;
}

u8 Menu_GetDifficulty(void)
{
    return selectedDifficulty;
}

u16 Menu_GetMaxHP(void)
{
    switch (selectedDifficulty) {
    case 1:  /* Hard */
        return 5u;
    case 0:  /* Normal */
    default:
        return 9u;
    }
}
