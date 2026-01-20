#include "stm32f10x.h"

void Delay(u32 count){
	u32 i;
	for (i=0; i<count; i++);
}


int main(void){
	//GPIO_InitTypeDef GPIO_InitStructure;
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //PB8=BUZZER, PB5=DS1(LED1)
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE); //PE2=KEY2
	
	RCC ->APB2ENR |= 1 << 3;	
	/*
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //LED
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_5);
	*/

	GPIOB->CRL &= 0xFF0FFFFF; //clear the 4bits for PB5
	GPIOB->CRL |= 0x00300000; //set MODE5=11, CNF5=00
	
	
	while (1){
		GPIOB->BRR = 1 << 5;
		Delay (1000000);
		GPIOB->BSRR = 1 << 5;
		Delay (1000000);
	}
	
}
