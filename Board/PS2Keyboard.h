// PS2Keyboard.h
#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include "stm32f10x.h"

/* Parsed PS2 key event: code (scan code), isBreak (release flag), isExtended (multi-byte prefix) */
typedef struct {
    u8 code;        /* Key scan code (0x00-0xFF), identifying which physical key */
    u8 isBreak;     /* 1 if key is released (break code 0xF0 prefix), 0 if pressed */
    u8 isExtended;  /* 1 if key is extended (0xE0 prefix like arrow keys), 0 if standard */
} PS2KeyEvent;

/* Initialize PS2 keyboard: reset FIFO buffers and configure external interrupt on PC11 */
void PS2_Init(void);

/* ISR for PS2 clock falling edge: collect bits and assemble 11-bit PS2 frames into rawFifo */
void PS2_OnClockFallingEdge(void);

/* Parse raw bytes from rawFifo: interpret 0xE0/0xF0 prefixes and generate PS2KeyEvent structures */
void PS2_Update(void);

/* Retrieve next parsed key event from event FIFO; returns 1 if successful, 0 if empty */
int  PS2_Poll(PS2KeyEvent *evt);

#endif

