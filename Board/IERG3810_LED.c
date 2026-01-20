#include "IERG3810_LED.h"

void IERG3810_LED_Init(void){
	
	RCC ->APB2ENR |= 1 << 3;	//Clock for GPIOB(PB5:LED0, PB8:BUZZER)
	RCC ->APB2ENR |= 1 << 6;  //Clock for GPIOE(PE2:KEY2, PE5:LED1) 
	
	
  GPIOB->CRL &= ~(0xF << (5*4)); //clear the 4bits for PB5
  GPIOB->CRL |=  (0x3 << (5*4)); //set MODE5=11 (50MHz), CNF5=00 (Push-Pull)
	
	GPIOE->CRL &= ~(0xF << (5*4)); //clear the 4bits for PE5
  GPIOE->CRL |=  (0x3 << (5*4)); //set MODE5=11 (50MHz), CNF5=00 (Push-Pull)

	DS0_OFF; DS1_OFF;
}
