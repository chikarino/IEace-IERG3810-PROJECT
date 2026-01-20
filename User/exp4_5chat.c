#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_LED.h"
#include "IERG3810_KEY.h"
#include "IERG3810_Buzzer.h"
#include "IERG3810_EXTI.h"
#include "IERG3810_NVIC.h"
#include "IERG3810_USART.h"
#include "Delay.h"

#define PS2_PREFIX_BREAK    0xF0
#define PS2_PREFIX_EXTENDED 0xE0
#define PS2_SCANCODE_KP0    0x70
#define PS2_SCANCODE_KP1    0x69

typedef void (*PS2_KeyHandler)(void);   /* Function pointer for key actions */

typedef struct{
  u8 scancode;                          /* PS/2 scan code (Set 2) */
  PS2_KeyHandler onPress;               /* Handler when key is pressed (make) */
  PS2_KeyHandler onRelease;             /* Handler when key is released (break) */
} PS2_KeyMapEntry;

typedef struct{
  volatile u8 dataReady;                /* Flag: ISR captured a complete byte */
  volatile u8 dataByte;                 /* Last received data byte */
  volatile u8 breakPending;             /* Next byte belongs to a break sequence */
  volatile u8 extendedPending;          /* Next byte is part of an extended key */
} PS2_FrameState;

volatile u8 ps2BitCount = 0;            /* Number of bits already sampled (0..10) */
volatile u16 ps2ShiftReg = 0;           /* Shift register storing start/data/parity/stop */
volatile PS2_FrameState ps2Frame = {0, 0, 0, 0};

/* ----- Key handlers (can be replaced or expanded later) ----- */
void PS2_Keypad0_Press(void){ DS0_ON; }
void PS2_Keypad0_Release(void){ DS0_OFF; }
void PS2_Keypad1_Press(void){ DS1_ON; }
void PS2_Keypad1_Release(void){ DS1_OFF; }

/* Lookup table: connect scan codes to their handlers */
PS2_KeyMapEntry ps2KeyTable[] = {
  { PS2_SCANCODE_KP0, PS2_Keypad0_Press, PS2_Keypad0_Release },
  { PS2_SCANCODE_KP1, PS2_Keypad1_Press, PS2_Keypad1_Release }
};
#define PS2_KEY_TABLE_SIZE (sizeof(ps2KeyTable) / sizeof(ps2KeyTable[0]))

/* ----------------------------------------------------------------
 * PS2_ProcessByte
 *  - Called in the main loop when ps2Frame.dataReady == 1.
 *  - Decodes prefixes (0xF0, 0xE0) and executes the mapped action.
 *  - Keeps logic simple and easy to reuse for new keys or outputs.
 * ----------------------------------------------------------------*/
static void PS2_ProcessByte(void){
  u8 dataByte;
  u8 i;

  if (ps2Frame.dataReady == 0){
    return;                              /* Nothing to process */
  }

  dataByte = ps2Frame.dataByte;          /* Copy byte from ISR buffer */
  ps2Frame.dataReady = 0;                /* Clear flag so ISR can set it again */

  if (dataByte == PS2_PREFIX_EXTENDED){
    ps2Frame.extendedPending = 1;        /* Remember next byte is extended key */
    return;
  }

  if (dataByte == PS2_PREFIX_BREAK){
    ps2Frame.breakPending = 1;           /* Next byte is the released key code */
    return;
  }

  /* Search the lookup table for a matching scan code */
  for (i = 0; i < PS2_KEY_TABLE_SIZE; i++){
    if (ps2KeyTable[i].scancode == dataByte){
      if (ps2Frame.breakPending != 0){
        if (ps2KeyTable[i].onRelease != 0){
          ps2KeyTable[i].onRelease();    /* Execute release handler */
        }
        ps2Frame.breakPending = 0;       /* Break handled → clear flag */
      }else{
        if (ps2KeyTable[i].onPress != 0){
          ps2KeyTable[i].onPress();      /* Execute press handler */
        }
      }
      ps2Frame.extendedPending = 0;      /* Ignore extended flag in this demo */
      return;
    }
  }
}

/* ----------------------------------------------------------------
 * main
 *  - Initializes peripherals, NVIC, and EXTIs.
 *  - Loops forever, delegating PS/2 handling to PS2_ProcessByte().
 * ----------------------------------------------------------------*/
int main(void){
  u32 sheep;

  clocktree_init();                      /* Configure system clocks */
  IERG3810_LED_Init();                   /* Prepare DS0 / DS1 outputs */
  IERG3810_KEY_Init();                   /* Prepare onboard keys */
  IERG3810_Buzzer_Init();                /* Prepare buzzer pin */

  nvic_setPriorityGroup(5);              /* 2-bit preempt + 2-bit sub priority */
  key2_extiInit(0x65);                   /* KEY2 interrupt priority */
  keyup_extiInit(0x75);                  /* KEYUP interrupt priority */
  ps2key_extiInit(0x10);                 /* PS/2 clock interrupt priority */

  DS0_OFF;                               /* Make sure LEDs start OFF */
  DS1_OFF;

  sheep = 0;
  while(1){
    sheep++;                             /* Idle counter (optional) */
    PS2_ProcessByte();                   /* Handle any pending PS/2 message */
  }
}

/* ----------------------------------------------------------------
 * EXTI15_10_IRQHandler
 *  - Triggered on every falling edge of the PS/2 clock (PC11 → EXTI11).
 *  - Samples the data line (PC10) bit by bit: start + 8 data + parity + stop.
 *  - Valid frames (start=0, stop=1) store data into ps2Frame and raise flag.
 * ----------------------------------------------------------------*/
void EXTI15_10_IRQHandler(void){
  u32 dataBit;
  u8 count;
  u8 stopBit;

  if ((EXTI->PR & (1<<11)) != 0){        /* Confirm EXTI11 caused the interrupt */
    dataBit = (GPIOC->IDR >> 10) & 0x1;  /* Sample PS/2 DATA line (PC10) */
    count = ps2BitCount;

    if (count == 0){
      if (dataBit != 0){
        ps2BitCount = 0;                 /* Invalid start bit (must be 0) → reset */
        ps2ShiftReg = 0;
        EXTI->PR = (1<<11);
        return;
      }
      ps2ShiftReg = 0;                   /* Clear shift register for new frame */
    }

    if (count < 11){
      ps2ShiftReg |= (u16)(dataBit << count); /* Store bit using LSB-first order */
      count++;
      ps2BitCount = count;

      if (count == 11){
        stopBit = (u8)((ps2ShiftReg >> 10) & 0x1); /* Extract stop bit */
        if (((ps2ShiftReg & 0x1) == 0) && (stopBit != 0)){
          ps2Frame.dataByte = (u8)((ps2ShiftReg >> 1) & 0xFF); /* Extract data */
          ps2Frame.dataReady = 1;        /* Notify main loop a byte is ready */
        }
        ps2BitCount = 0;                 /* Reset state for next packet */
        ps2ShiftReg = 0;
      }
    }
    EXTI->PR = (1<<11);                  /* Clear EXTI11 pending flag */
  }
}
