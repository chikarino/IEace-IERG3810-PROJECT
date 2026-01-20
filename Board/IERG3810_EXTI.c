#include "IERG3810_EXTI.h"

void key2_extiInit(u8 priority){
  //KEY2 at PE2, EXTI-2, IRQ#8
  RCC->APB2ENR |= 1<<6; // Enable Port-E clock
  GPIOE->CRL &= 0xFFFFF0FF; //rm0008v21 c21 p171, modifty PE2
  GPIOE->CRL |= 0x00000800; // CNF=10 (input with pull-up/down), MODE=00
  GPIOE->ODR |= 1<<2; //pull-up
  RCC->APB2ENR |= 0x01; //AFIOEN, RCC_APB2ENR(rm0008 p146)
  AFIO->EXTICR[0] &= 0xFFFFF0FF; //AFIO_EXTICR1(rm0008 p191)
  AFIO->EXTICR[0] |= 0x00000400;
  EXTI->IMR |= 1<<2; //MR2 (RM0008 p211) edge trigger
  EXTI->FTSR |= 1<<2; //TR2 (RM0008 p212)falling edge trigger 
  //EXTI->RTSR |= 1<<2; //TR2 (RM0008 p212)rising edge trigger
    EXTI->PR    = (1<<2); //clear pending bit
  NVIC->IP[8] = priority; //set priority of IRQ#8
    //DDI0337E page-8-3
  NVIC->ISER[0] |= 1<<8; //enable NVIC IRQ#8
}

void keyup_extiInit(u8 priority){
  //KEY UP at PA0, EXTI-0, IRQ#6
  RCC->APB2ENR |= 1<<2; // Enable Port-A clock
  GPIOA->CRL &= 0xFFFFFFF0; //rm0008v21 c21 p171, modifty PA0
  GPIOA->CRL |= 0x00000008; // CNF=10 (input with pull-up/down), MODE=00
  GPIOA->ODR &= ~(1<<0); //pull-down
  RCC->APB2ENR |= 0x01; //AFIOEN, RCC_APB2ENR(rm0008 p146)
  AFIO->EXTICR[0] &= 0xFFFFFFF0; //AFIO_EXTICR1(rm0008 p191)
  AFIO->EXTICR[0] |= 0x00000000;
  EXTI->IMR |= 1<<0; //MR0 (RM0008 p211) edge trigger
  //EXTI->FTSR |= 1<<0; //TR0 (RM0008 p212)falling edge trigger 
    EXTI->FTSR &= ~(1<<0);  //Comfirm no falling edge trigger
  EXTI->RTSR |= 1<<0; //TR0 (RM0008 p212)rising edge trigger
    EXTI->PR    = (1<<0); //clear pending bit
  NVIC->IP[6] = priority; //set priority of IRQ#6
    //DDI0337E page-8-3
  NVIC->ISER[0] |= 1<<6; //enable NVIC IRQ#6
}


void ps2key_extiInit(u8 priority){
  // PS/2 Clock: PC11 → EXTI11 → IRQ#41 (EXTI15_10_IRQn)
  RCC->APB2ENR |= (1<<4);           // Enable Port-C Clock
  GPIOC->CRH  &= ~(0xFu << 12);     // Clear CRH[15:12]（PC11）
  //GPIOC->CRH  |=  (0x4u << 12);     // CNF=01 (Float input)，MODE=00
  // 若要使用上拉，下拉，請改成 CNF=10 + ODR 設為 1（上拉)
  GPIOC->CRH |=  (0x4u << 12);     // CNF=01 (Float input)，MODE=00
  GPIOC->ODR |= (1<<11);               // Set PC11 to pull-up

  RCC->APB2ENR |= (1<<0);           // AFIOEN
  AFIO->EXTICR[2] &= ~(0xFu << 12); // EXTICR3[15:12] -> EXTI11
  AFIO->EXTICR[2] |=  (0x2u << 12); // 0x2 → Port C

  EXTI->IMR  |=  (1<<11);           // Unmask EXTI11 edge trigger
  EXTI->FTSR |=  (1<<11);           // PS/2 clock Falling edge trigger
  EXTI->RTSR &= ~(1<<11);           // No Rising edge trigger
  EXTI->PR    =  (1<<11);           // Clear pending bit

  NVIC->IP[EXTI15_10_IRQn] = priority;
  NVIC->ISER[1] |= (1 << (EXTI15_10_IRQn - 32)); // IRQ#41 → ISER[1] bit9
}
