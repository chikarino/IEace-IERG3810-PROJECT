#include "stm32f10x.h"

void Delay(u32 count){
	u32 i;
	for (i=0; i<count; i++);
}

void clocktree_init(void)
{
  // 清必要欄位（不細述）
  RCC->CFGR &= 0xF8FF0000;
  RCC->CR   &= 0xFFE6FFFF;

  // HSE ON + 等 HSERDY
  RCC->CR   |= (1u<<16);
  while ((RCC->CR & (1u<<17)) == 0) { }

  // AHB=/1, APB1=/2, APB2=/1
  RCC->CFGR  = 0x00000400;  // PPRE1=100b (/2)

  // PLLSRC=HSE, PLLMUL=×9
  RCC->CFGR |= (1u<<16);
  RCC->CFGR |= (7u<<18);    // 0b0111→×9

  // FLASH：2WS + Prefetch
  FLASH->ACR &= ~7u;        // 清 LATENCY
  FLASH->ACR |=  2u;        // 2WS
  FLASH->ACR |= (1u<<4);    // PRFTBE

  // PLL ON + 等 PLLRDY
  RCC->CR   |= (1u<<24);
  while ((RCC->CR & (1u<<25)) == 0) { }

  // 切 SYSCLK=PLL + 等 SWS=PLL
  RCC->CFGR = (RCC->CFGR & ~3u) | 2u;
  while (((RCC->CFGR>>2)&3u) != 2u) { }
}

void usart2_init(u32 pclk1, u32 baud)
{
  // 計算 BRR（與你原本一致）
  float t = (float)(pclk1 * 1000000u) / (baud * 16u);
  u16 mantissa = (u16)t;
  u16 fraction = (u16)((t - mantissa) * 16.0f);
  u16 brr      = (u16)((mantissa << 4) | (fraction & 0xF));

  // 開時鐘：AFIO（保險）、GPIOA、USART2
  RCC->APB2ENR |= (1u<<0);   // AFIOEN（可選，但建議）
  RCC->APB2ENR |= (1u<<2);   // GPIOAEN
  RCC->APB1ENR |= (1u<<17);  // USART2EN

  // PA2: AF PP 50MHz；PA3: Input floating
  GPIOA->CRL &= 0xFFFF00FF;
  GPIOA->CRL |= 0x00000B00;  // PA2
  GPIOA->CRL |= 0x00004000;  // PA3 (浮空)

  // 軟重置 USART2
  RCC->APB1RSTR |=  (1u<<17);
  RCC->APB1RSTR &= ~(1u<<17);

  // 設鮑率 + 開 UE/TE
  USART2->BRR = brr;
  USART2->CR1 = (1u<<13) | (1u<<3); // UE|TE
}


int main (void)
{
  clocktree_init();           
  usart2_init(36, 9600); 
  Delay(2000000); 
	

  while(1)
  {
    USART2->DR = 0x41; 
		Delay(500000);
    USART2->DR = 0x42;
    Delay(50000); 
    Delay(1000000);
  }
}
