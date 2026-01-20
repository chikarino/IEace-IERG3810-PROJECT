#include "Audio.h"

/* 
 * USE TIM4 Channel 3 to drive PB8 buzzer output
 * Generate different frequency square waves via PWM to drive an active buzzer
 */

/* BGM playback state */
static const Note* g_bgmNotes = 0;     /* Current BGM note array */
static Note g_bgmScaledNotes[512];     /* Scaled note array (up to 512 notes) */
static u16 g_bgmNoteCount = 0;         /* Total number of notes */
static u16 g_bgmCurrentNote = 0;       /* Current playing note index */
static u16 g_bgmTimer = 0;             /* Current note timer */
static u8 g_bgmLoop = 0;               /* Whether to loop playback */
static u8 g_bgmPlaying = 0;            /* Whether BGM is playing */

void Audio_Init(void)
{
    /* 1. Enable clocks */
    RCC->APB1ENR |= (1 << 0);   /* TIM2 clock */
    RCC->APB2ENR |= (1 << 0);   /* AFIO clock */
    RCC->APB2ENR |= (1 << 3);   /* GPIOB clock */
    
    /* 2. 
     *    To simplify, we switch to using TIM4_CH3 to drive PB8
     */
    
    /* Switch to TIM4: */
    RCC->APB1ENR |= (1 << 2);   /* TIM4 clock */
    
    /* 3. Configure PB8 as alternate function push-pull output 50MHz */
    GPIOB->CRH &= ~(0xFu << 0);  /* Clear PB8 configuration */
    GPIOB->CRH |=  (0xBu << 0);  /* MODE8=11(50MHz), CNF8=10(AF Push-Pull) */
    
    /* 4. Configure TIM4 */
    TIM4->CR1 = 0x0000;
    TIM4->CR1 |= (1 << 7);      /* ARPE: Auto-reload preload enable */
    
    /* Default to mute (frequency=0, ARR will be set in Audio_PlayTone) */
    TIM4->ARR = 1000;
    TIM4->PSC = 71;             /* 72MHz / (71+1) = 1MHz time base */
    
    /* 5. Configure Channel 3 as PWM mode (PB8 = TIM4_CH3) */
    TIM4->CCMR2 &= ~(0x7 << 4);  /* Clear OC3M */
    TIM4->CCMR2 |=  (0x6 << 4);  /* PWM mode 1 (110) */
    TIM4->CCMR2 |=  (1 << 3);    /* OC3PE: Preload enable */
    
    TIM4->CCR3 = 0;              /* Initial duty cycle=0 (mute) */
    
    TIM4->CCER |= (1 << 8);      /* CC3E: Enable CH3 output */
    
    /* 6. Generate update event and start counter */
    TIM4->EGR |= 1;              /* UG: Update generation */
    TIM4->CR1 |= 0x01;           /* CEN: Counter enable */
}

void Audio_PlayTone(u16 frequency_hz)
{
    u32 arr_val;
    u32 adjusted_freq;
    
    if (frequency_hz == 0) {
        Audio_Stop();
        return;
    }
    
    /* 
     * Active buzzer frequency adjustment
     */
    if (frequency_hz < 1000) {
        adjusted_freq = (u32)frequency_hz * 4;
    } else if (frequency_hz < 2000) {
        adjusted_freq = (u32)frequency_hz * 5;
    } else {
        adjusted_freq = (u32)frequency_hz * 5;
    }
    
    /* Limit frequency to a reasonable range */
    if (adjusted_freq > 10000) adjusted_freq = 10000;
    if (adjusted_freq < 1200) adjusted_freq = 1200;
    

    
    /* 
     * Calculate ARR value: 
     * Time base frequency = 72MHz / (PSC+1) = 72MHz / 72 = 1MHz
     * ARR = Time base frequency / Adjusted frequency - 1
     */
    arr_val = 1000000UL / adjusted_freq;
    if (arr_val > 0) arr_val -= 1;
    if (arr_val > 65535) arr_val = 65535;  /* Limit to 16-bit range */
    
    TIM4->ARR = (u16)arr_val;
    TIM4->CCR3 = (u16)(arr_val * 0.75);  /* 75% duty cycle */
    TIM4->EGR |= 1;  /* Update registers */
}

void Audio_Stop(void)
{
    TIM4->CCR3 = 0;  /* Duty cycle set to 0, stop output */
}


/* Start playing BGM */
void Audio_StartBGM(const Note* notes, u16 noteCount, u8 loop)
{
    u16 i;
    
    if (notes == 0 || noteCount == 0) return;
    if (noteCount > 512) noteCount = 512;  /* Limit maximum number of notes */
    
    /* 
     * Scale duration from 60fps to 100fps (10ms update rate)
     * 60fps = 16.67ms/frame
     * 10ms update = 100fps
     * scale factor = 100/60 = 5/3 ≈ 1.667
     * Using integer arithmetic: duration_scaled = (duration * 5 + 2) / 3
     * Adding 2 for rounding
     */
    for (i = 0; i < noteCount; i++) {
        g_bgmScaledNotes[i].frequency = notes[i].frequency;
        g_bgmScaledNotes[i].duration = (notes[i].duration * 5u + 2u) / 3u;
    }
    
    g_bgmNotes = g_bgmScaledNotes;  /* Use scaled version */
    g_bgmNoteCount = noteCount;
    g_bgmCurrentNote = 0;
    g_bgmTimer = 0;
    g_bgmLoop = loop;
    g_bgmPlaying = 1;
    
    /* Don't play the first note immediately - let Audio_UpdateBGM handle it */
    /* This avoids timing issues at startup */
}

/* Stop BGM */
void Audio_StopBGM(void)
{
    g_bgmPlaying = 0;
    Audio_Stop();
}

/* Update BGM every frame (should be called in the game loop) */
void Audio_UpdateBGM(void)
{
    const Note* currentNote;
    
    if (!g_bgmPlaying) return;
    if (g_bgmNotes == 0) return;
    
    /* Get current note */
    currentNote = &g_bgmNotes[g_bgmCurrentNote];
    
    /* On the first frame (timer=0), start playing the first note */
    if (g_bgmTimer == 0) {
        if (currentNote->frequency > 0) {
            Audio_PlayTone(currentNote->frequency);
        } else {
            Audio_Stop();  /* Rest */
        }
    }
    
    /* Increment timer */
    g_bgmTimer++;
    
    /* Check if the current note has finished playing */
    if (g_bgmTimer >= currentNote->duration) {
        /* Switch to the next note */
        g_bgmCurrentNote++;
        g_bgmTimer = 0;
        
        /* Check if the entire BGM has been played */
        if (g_bgmCurrentNote >= g_bgmNoteCount) {
            if (g_bgmLoop) {
                /* Loop playback, return to the beginning */
                g_bgmCurrentNote = 0;
            } else {
                /* No loop, stop playback */
                Audio_StopBGM();
                return;
            }
        }
        
        /* Play the new note */
        currentNote = &g_bgmNotes[g_bgmCurrentNote];
        if (currentNote->frequency > 0) {
            Audio_PlayTone(currentNote->frequency);
        } else {
            Audio_Stop();  /* Rest */
        }
    }
}
