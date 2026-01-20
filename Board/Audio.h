#ifndef __AUDIO_H
#define __AUDIO_H

#include "stm32f10x.h"

/* Note structure */
typedef struct {
    u32 frequency;  /* Frequency (Hz), 0 means rest */
    u32 duration;   /* Duration (frames, 60fps) */
} Note;

/* Audio system initialization - Use TIM4 PWM to control buzzer (PB8) */
void Audio_Init(void);

/* Play sound effect with specified frequency */
void Audio_PlayTone(u16 frequency_hz);

/* Stop playing */
void Audio_Stop(void);

/* BGM system */
void Audio_StartBGM(const Note* notes, u16 noteCount, u8 loop);  /* Start playing BGM */
void Audio_StopBGM(void);                                         /* Stop BGM */
void Audio_UpdateBGM(void);                                       /* Update every frame (should be called in the game loop) */

#endif
