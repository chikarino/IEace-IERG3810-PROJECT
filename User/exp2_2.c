#include "stm32f10x.h"
void Delay(u32 count);
void clocktree_init(void);
void usart2_init(u32 pclk1, u32 baud);
void usart1_init (u32 pclk1, u32 baud);
void usart_print(u8 USARTport, char *s);
void usart_print_txe(u8 USARTport, char *s);


void Delay(u32 count){
  u32 i;
  for (i=0; i<count; i++);
}

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

// Goal: Initialize USART2 at the given baud rate (e.g., 36 MHz / 9600 bps -> BRR = 0x0EA6), TX only (TE).
// Ref: RM0008  -- USART->BRR, USART->CR1, GPIOx_CRL, RCC APB1/2 ENR & RSTR
void usart2_init(u32 pclk1, u32 baud)
{
  // -------- Compute BRR (USART_BRR) --------
  // USARTDIV = PCLK1 / (16 * baud)
  // BRR = (mantissa << 4) | fraction, where fraction = fractional part of USARTDIV × 16.
  float temp;                  // Holds USARTDIV (float)
  u16   mantissa;              // Integer part -> BRR[15:4]
  u16   fraction;              // Fraction × 16 -> BRR[3:0]

  // pclk1 is passed in MHz
  temp = (float)(pclk1 * 1000000) / (baud * 16);

  mantissa = (u16)temp;                       // Integer part
  fraction = (u16)((temp - mantissa) * 16);   // Fractional part ×16 (truncation)
  mantissa <<= 4;                             // Move into BRR[15:4]
  mantissa += fraction & 0xF;                 // Add BRR[3:0] (mask for safety)

  // -------- Enable peripheral clocks --------
  RCC->APB2ENR |= 1 << 2;                     // GPIOAEN (APB2ENR bit2)
  RCC->APB1ENR |= 1 << 17;                    // USART2EN (APB1ENR bit17)

  // -------- Configure PA2/PA3 pins --------
  // CRL controls pins PA0..PA7; pin2 uses bits [11:8], pin3 uses bits [15:12].
  // Each pin uses 4 bits: CNF[1:0] + MODE[1:0].
  GPIOA->CRL &= 0xFFFF00FF;                   // Clear 8 bits for pin2 and pin3

  // 0x00008B00 sets:
  //   pin2: 0xB (1011b) = CNF2=10 (AF push pull) + MODE2=11 (50 MHz)
  //   pin3: 0x8 (1000b) = CNF3=10 (input with pull up/pull down) + MODE3=00 (input)
  GPIOA->CRL |= 0x00008B00;

  // -------- Soft reset USART2 (clean state) --------
  RCC->APB1RSTR |=  1 << 17;                  // Set to enter reset
  RCC->APB1RSTR &= ~(1 << 17);                // Clear to exit reset

  // -------- Set baud rate and enable TX --------
  USART2->BRR = mantissa;                     // e.g., pclk1=36, baud=9600 -> 0x0EA6
  USART2->CR1 = 0x2008;                       // UE(bit13)=1 (USART enable), TE(bit3)=1 (TX enable), default 8N1
}

void usart1_init(u32 pclk2, u32 baud)
{
  // --------BRR Calculation--------
  float temp;
  u16   mantissa;
  u16   fraction;

  temp     = (float)(pclk2 * 1000000u) / (baud * 16u);  // pclk2 in MHz 
  mantissa = (u16)temp;
  fraction = (u16)((temp - mantissa) * 16.0f);
  mantissa = (mantissa << 4) + (fraction & 0xF);        // Combine to a complete BRR

  // -------- enable clock USART1 under APB2--------
  RCC->APB2ENR |= (1u << 2);    // GPIOAEN
  RCC->APB2ENR |= (1u << 14);   // USART1EN

  // -------- pins：PA9=TX、PA10=RX --------
  // CRH pin8~15；pin9 => [7:4]，pin10 => [11:8]
  GPIOA->CRH &= ~((0xFu << 4) | (0xFu << 8));   // clear the 2 nibbles
  GPIOA->CRH |= (0xB  << 4);                    // PA9: 0xB = AF PP, 50 MHz
  GPIOA->CRH |= (0x4  << 8);                    // PA10: 0x4 = Input floating

  // -------- Reset USART1--------
  RCC->APB2RSTR |=  (1u << 14);
  RCC->APB2RSTR &= ~(1u << 14);

  // -------- set BRR and Enable UE/TE --------
  USART1->BRR = mantissa;       // 72MHz/9600 → 0x1D4C
  USART1->CR1 = 0x2008;         // UE=1、TE=1 (enable, transmit)
}


void usart_print(u8 USARTport, char *st)
{
  u8 i = 0;       
  while (st[i] != 0x00)              // Ends with 0x00 
  {
    if (USARTport == 1) USART1->DR = (u16)st[i];
    if (USARTport == 2) USART2->DR = (u16)st[i];

    Delay(50000);
    if (i == 255) break;             // Max length 255
    i++;
  }
}

void usart_print_txe(u8 USARTport, char *st)
{
  u8 i = 0; 
  while (st[i] != 0x00) //ends with 0x00
  {
    if (USARTport == 1)
    {
      while ((USART1->SR & (1u<<7)) == 0) { }   // wait for USART1->SR  TXE(bit7)=1
      USART1->DR = (u16)st[i];
    }
    if (USARTport == 2)
    {
      while ((USART2->SR & (1u<<7)) == 0) { }   // wait for USART2->SR  TXE(bit7)=1
      USART2->DR = (u16)st[i];
    }

    if (i == 255) break;                         // 256 max
    i++;
  }

}

int main (void)
{
	
	const char id[] = "1155208116\r\n";
	u32 i = 0;
  clocktree_init();           
  usart2_init(36, 9600); 
	usart1_init(72, 9600);       // USART1：PCLK2=72MHz、9600-8N1
  Delay(2000000); 
  for (i=0 ; id[i] != '\0'; ++i) {	//to TDR
    USART1->DR = (u16)id[i];
    Delay(100000); 
  }
  while(1)
  {

    USART2->DR = 0x41; 
		Delay (500000);
		Delay(100);
    USART2->DR = 0x42;
    Delay(50000); 
    Delay(1000000);
  }
}
