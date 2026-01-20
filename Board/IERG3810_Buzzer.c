#include "IERG3810_Buzzer.h"

void IERG3810_Buzzer_Init(void){

		RCC ->APB2ENR |= 1 << 3;	//Clock for GPIOB(PB5:LED0, PB8:BUZZER)
		GPIOB->CRH &= ~(0xFu << (0*4)); //clear the 4bits for PB8
		GPIOB->CRH |=  (0x3u << (0*4)); //set  MODE8=11 (50MHz), CNF=00 (Push-Pull)
		BUZ_OFF;
}
