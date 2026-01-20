#include "IERG3810_Clock.h"

// Goal: HSE (8 MHz) -> PLL ×9 -> SYSCLK = 72 MHz; AHB = 72 MHz; APB1 = 36 MHz; APB2 = 72 MHz
// Ref: RM0008 -- RCC->CR, RCC->CFGR, FLASH->ACR
void clocktree_init(void)
{
  u8 PLL  = 7;                 // PLLMUL field value: 0b0111 means ×9 (CFGR PLLMUL[21:18])
  unsigned char temp = 0;      // Temp for polling SWS[1:0] (system clock switch status)

  // Put selected CFGR fields into a known state.
  // 0xF8FF0000 Clears bit0-15, 24-26
  RCC->CFGR &= 0xF8FF0000;

  // 0xFEF6FFFF clears bit24 PLLON, bit19 CSSON, bit16 HSEON
  RCC->CR   &= 0xFEF6FFFF;

  // Turn on the external high-speed oscillator (HSE, 8 MHz)
  RCC->CR   |= 0x00010000;     // HSEON = 1 (bit16)

  // Wait for HSE ready (HSERDY = 1, bit17).
  // RCC->CR & (1u<<17))
  while (!(RCC->CR >> 17));

  // APB1 prescaler: /2 (PCLK1 max 36 MHz). Keep others at /1.
  RCC->CFGR  = 0x00000400;     // PPRE1[10:8] = 100b -> /2

  // Set PLL multiplier to ×9 (PLLMUL[21:18])
  RCC->CFGR |= PLL << 18;

  // Select HSE as PLL source (PLLSRC = 1)
  RCC->CFGR |= 1 << 16;
  RCC->CFGR |= 1 << 16; 

  // FLASH access: 2 wait states + prefetch enable (required at 72 MHz)
  // 0x32 = LATENCY[2:0]=010b (2WS) + PRFTBE(bit4)=1. PRFTBS(bit5) is status; writing it has no effect.
  FLASH->ACR |= 0x32;

  // Enable PLL
  RCC->CR   |= 0x01000000;     // PLLON = 1 (bit24)

  // Wait for PLL ready (PLLRDY = 1, bit25)
  while (!(RCC->CR >> 25));

  // Switch system clock to PLL (SW = 10)
  RCC->CFGR |= 0x00000002;

  // Confirm the switch: SWS[3:2] must read 10 (SYSCLK from PLL)
  while (temp != 0x02) {
    temp  = RCC->CFGR >> 2;    // Move SWS[3:2] to bits [1:0]
    temp &= 0x03;              // Keep the last two bits
  }

  // Now: HCLK = 72 MHz, PCLK1 = 36 MHz, PCLK2 = 72 MHz
}
