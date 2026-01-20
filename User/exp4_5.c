#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_LED.h"
#include "IERG3810_KEY.h"
#include "IERG3810_Buzzer.h"
#include "IERG3810_EXTI.h"
#include "IERG3810_NVIC.h"
#include "IERG3810_USART.h"
#include "Delay.h"

#define DS1_toggle do{ ds1_on ^= 1; if(ds1_on) DS1_ON; else DS1_OFF; }while(0)
#define Buzzer_toggle do{ buz_on ^= 1; if(buz_on) BUZ_ON; else BUZ_OFF; }while(0)

#define PS2_SCANCODE_KP0 0x70   // Keypad 0 make code
#define PS2_SCANCODE_KP1 0x69   // Keypad 1 make code
volatile u8 ps2BitCount = 0;
volatile u16 ps2ShiftReg = 0;
volatile u8 ps2DataReady = 0;
volatile u8 ps2DataByte = 0;
volatile u8 ps2BreakPending = 0;
volatile u8 ps2ExtendedPending = 0;

// EXTI handler for KEYUP
void EXTI0_IRQHandler(void){
  u8 i;
  if ((EXTI->PR & (1<<0)) != 0){
    for (i = 0; i < 10; i++){
      DS1_ON;
      Delay(1000000);
      DS1_OFF;
      Delay(1000000);
    }
    EXTI->PR = (1<<0);
  }
}

// EXTI handler for KEY2
void EXTI2_IRQHandler(void){
  u8 i;
  if ((EXTI->PR & (1<<2)) != 0){
    for (i = 0; i < 10; i++){
      DS0_ON;
      Delay(1000000);
      DS0_OFF;
      Delay(1000000);
    }
    EXTI->PR = (1<<2);
  }
}

// EXTI handler for PS/2 clock (PC11 → EXTI11)
void EXTI15_10_IRQHandler(void){
  u32 dataBit;
  u8 count;
  u8 stopBit;

  if ((EXTI->PR & (1<<11)) != 0){     // Ensure EXTI11 triggered the interrupt
    dataBit = (GPIOC->IDR >> 10) & 0x1; // Read PS/2 data line (PC10)
    count = ps2BitCount;

    if (count == 0){
      if (dataBit != 0){
        ps2BitCount = 0;
        ps2ShiftReg = 0;
        EXTI->PR = (1<<11);          // Invalid start bit → abort
        return;
      }
      ps2ShiftReg = 0;               // Clear shift register for a new frame
    }

    if (count < 11){
      ps2ShiftReg |= (u16)(dataBit << count); // Latch bit using LSB-first order
      count++;
      ps2BitCount = count;

      if (count == 11){
        stopBit = (u8)((ps2ShiftReg >> 10) & 0x1); // Extract stop bit
        if (((ps2ShiftReg & 0x1) == 0) && (stopBit != 0)){
          ps2DataByte = (u8)((ps2ShiftReg >> 1) & 0xFF); // Extract data byte
          ps2DataReady = 1;        // Flag: data ready for main loop
        }
        ps2BitCount = 0;           // Reset state for next frame
        ps2ShiftReg = 0;
      }
    }

    EXTI->PR = (1<<11);            // Clear EXTI11 pending bit
  }
}

int main(void){
  u32 sheep;
  u8 dataByte;
  sheep = 0;  //What
  /*IO INIT*/
  clocktree_init(); 
  IERG3810_LED_Init();
  IERG3810_KEY_Init();
  IERG3810_Buzzer_Init(); 
  /*********/
  nvic_setPriorityGroup(5);
  key2_extiInit(0x65); 
  keyup_extiInit(0x75); 
  ps2key_extiInit(0x10); 

  DS0_OFF;     
  DS1_OFF;

  while(1){
    sheep++;
    if (ps2DataReady != 0){   // Execute if a PS/2 byte is ready
      dataByte = ps2DataByte;
      ps2DataReady = 0;

      if (dataByte == 0xE0){
        ps2ExtendedPending = 1;  // Extended scancode prefix
      }else if (dataByte == 0xF0){
        ps2BreakPending = 1;     // Break-code prefix
      }else{
        if (ps2BreakPending != 0){
          if (dataByte == PS2_SCANCODE_KP0){
            DS0_OFF;             // Release keypad 0 → turn off DS0
          }else if (dataByte == PS2_SCANCODE_KP1){
            DS1_OFF;             // Release keypad 1 → turn off DS1
          }
          ps2BreakPending = 0;   // Clear break flag after handling
        }else{
          if (dataByte == PS2_SCANCODE_KP0){
            DS0_ON;              // Press keypad 0 → turn on DS0
          }else if (dataByte == PS2_SCANCODE_KP1){
            DS1_ON;              // Press keypad 1 → turn on DS1
          }
        }
        ps2ExtendedPending = 0;
      }
    }
  }
}
