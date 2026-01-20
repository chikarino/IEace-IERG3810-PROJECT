#include "stm32f10x.h"

void IERG3810_Buzzer_Init(void);

#define BUZ_ON   (GPIOB->BSRR = (1u<<8)) //BUZZER
#define BUZ_OFF  (GPIOB->BRR  = (1u<<8))
