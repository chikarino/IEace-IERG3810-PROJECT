#include "stm32f10x.h"

void IERG3810_KEY_Init(void);
u8 IERG3810_KEY1_ReadEdge(void);
u8 IERG3810_KEY2_ReadEdge(void);
u8 IERG3810_KEYUP_ReadEdge(void);



#define KEY2_DOWN    ((GPIOE->IDR & (1u<<2)) == 0) 
#define KEY1_DOWN   ((GPIOE->IDR & (1u<<3)) == 0)  
#define KEYUP_DOWN  ((GPIOA->IDR & (1u<<0)) != 0)
