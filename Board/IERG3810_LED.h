#include "stm32f10x.h"

void IERG3810_LED_Init(void);

#define DS0_ON   (GPIOB->BRR  = (1u<<5)) //LED0
#define DS0_OFF  (GPIOB->BSRR = (1u<<5))
#define DS1_ON   (GPIOE->BRR  = (1u<<5)) //LED1
#define DS1_OFF  (GPIOE->BSRR = (1u<<5))
