#include "IERG3810_KEY.h"
#ifndef IERG3810_KEY_DEBOUNCE_GUARD
#define IERG3810_KEY_DEBOUNCE_GUARD 9000u
#endif

void IERG3810_KEY_Init(void){
	RCC ->APB2ENR |= 1 << 6;  //Clock for GPIOE(PE2:KEY2, PE5:LED1, PE3:KEY1) 
	RCC ->APB2ENR |= 1 << 2;  //Clock for GPIOA(PA0: KEY UP) 
	
	GPIOA->CRL &= ~(0xF << (0*4)); //clear the 4bits for PA0
  GPIOA->CRL |=  (0x8 << (0*4));  // set Mode0=00 (INPUT), CNF0=10 (Input with pull-up/pull-down)
	GPIOA->BRR = (1<<0); //selcet pull-down
	
	GPIOE->CRL &= ~(0xF << (2*4)); //clear the 4bits for PE2
  GPIOE->CRL |=  (0x8 << (2*4));  // set Mode2=00 (INPUT), CNF2=10 (Input with pull-up/pull-down)
	GPIOE->BSRR =  (1<<2); //selcet pull-up
	
	GPIOE->CRL &= ~(0xFu << (3*4)); //clear the 4bits for PE3
	GPIOE->CRL |=  (0x8u << (3*4));	//set Mode3=00 (INPUT), CNF3=10 (Input with pull-up/pull-down)
	GPIOE->BSRR = (1u<<3);	//selcet pull-up
}

u8 IERG3810_KEY1_ReadEdge(void){
  static u8  prev  = 0;      
  static u32 guard = 0;     

  uint8_t now = ((GPIOE->IDR & (1u<<3)) == 0) ? 1u : 0u;

  if (guard) guard--;
  if (!prev && now && guard==0){
    guard = IERG3810_KEY_DEBOUNCE_GUARD;
    prev  = now;
    return 1;    //return 1 if edge
  }
  prev = now;
  return 0;
}



u8 IERG3810_KEY2_ReadEdge(void){
  static u8  prev  = 0;
  static u32 guard = 0;
  u8 now = ((GPIOE->IDR & (1u<<2)) == 0) ? 1u : 0u;  
  if (guard) guard--;
  if (!prev && now && guard==0){ guard = IERG3810_KEY_DEBOUNCE_GUARD; prev = now; return 1; }
  prev = now; return 0;
}

u8 IERG3810_KEYUP_ReadEdge(void){
  static u8  prev  = 0;
  static u32 guard = 0;
  u8 now = ((GPIOA->IDR & (1u<<0)) != 0) ? 1u : 0u;
  if (guard) guard--;
  if (!prev && now && guard==0){ guard = IERG3810_KEY_DEBOUNCE_GUARD; prev = now; return 1; }
  prev = now; return 0;
}
