#include "stm32f10x.h"


u8 key1_press = 0;
u8 keyup_press = 0;
u8 ds1_on = 0, buz_on = 0;

void Delay(u32 count){
	u32 i;
	for (i=0; i<count; i++);
}
#define DEBOUNCE_GUARD  9000

#define DS0_ON   (GPIOB->BRR  = (1u<<5)) //LED0
#define DS0_OFF  (GPIOB->BSRR = (1u<<5))
#define DS1_ON   (GPIOE->BRR  = (1u<<5)) //LED1
#define DS1_OFF  (GPIOE->BSRR = (1u<<5))
#define BUZ_ON   (GPIOB->BSRR = (1u<<8)) //BUZZER
#define BUZ_OFF  (GPIOB->BRR  = (1u<<8))

#define KEY2_DOWN    ((GPIOE->IDR & (1u<<2)) == 0) 
#define KEY1_DOWN   ((GPIOE->IDR & (1u<<3)) == 0)  
#define KEYUP_DOWN  ((GPIOA->IDR & (1u<<0)) != 0)


#define DS1_toggle      do{ ds1_on ^= 1; if(ds1_on) DS1_ON; else DS1_OFF; }while(0)
#define Buzzer_toggle   do{ buz_on ^= 1; if(buz_on) BUZ_ON; else BUZ_OFF; }while(0)


void key1_update(void){
  static u8  prev = 0;
  static u32 guard = 0;
  u8 now = KEY1_DOWN;              
  if (guard) guard--;
  if (!prev && now && guard==0){      
		key1_press = 1;
    guard = DEBOUNCE_GUARD;
  }
  prev = now;                      
}


void keyup_update(void){
  static u8  prev = 0;
  static u32 guard = 0;
  u8 now = KEYUP_DOWN;               
  if (guard) guard--;
  if (!prev && now && guard==0){     
    keyup_press = 1;
    guard = DEBOUNCE_GUARD;
  }
  prev = now;
}




int main(void){
	RCC ->APB2ENR |= 1 << 3;	//Clock for GPIOB(PB5:LED0, PB8:BUZZER)
	RCC ->APB2ENR |= 1 << 6;  //Clock for GPIOE(PE2:KEY2, PE5:LED1, PE3:KEY1) 
	RCC ->APB2ENR |= 1 << 2;  //Clock for GPIOA(PA0: KEY UP) 
	
	GPIOA->CRL &= ~(0xF << (0*4)); //clear the 4bits for PA0
  GPIOA->CRL |=  (0x8 << (0*4));  // set Mode0=00 (INPUT), CNF0=10 (Input with pull-up/pull-down)
	GPIOA->BRR = (1<<0); //selcet pull-down

  GPIOB->CRL &= ~(0xF << (5*4)); //clear the 4bits for PB5
  GPIOB->CRL |=  (0x3 << (5*4)); //set MODE5=11 (50MHz), CNF5=00 (Push-Pull)
	
	GPIOB->CRH &= ~(0xF << (0*4)); //clear the 4bits for PB8
	GPIOB->CRH |=  (0x3 << (0*4)); //set  MODE8=11 (50MHz), CNF=00 (Push-Pull)
	
  GPIOE->CRL &= ~(0xF << (5*4)); //clear the 4bits for PE5
  GPIOE->CRL |=  (0x3 << (5*4)); //set MODE5=11 (50MHz), CNF5=00 (Push-Pull)
	
	GPIOE->CRL &= ~(0xF << (2*4)); //clear the 4bits for PE2
  GPIOE->CRL |=  (0x8 << (2*4));  // set Mode2=00 (INPUT), CNF2=10 (Input with pull-up/pull-down)
	GPIOE->BSRR =  (1<<2); //selcet pull-up
	
	GPIOE->CRL &= ~(0xFu << (3*4)); //clear the 4bits for PE3
	GPIOE->CRL |=  (0x8u << (3*4));
	GPIOE->BSRR = (1u<<3);
	
	
	
	while (1){
    if (KEY2_DOWN) DS0_ON; else DS0_OFF;
		
		key1_update();
    if (key1_press){ key1_press = 0; DS1_toggle; } 
		
	  keyup_update();
    if (keyup_press){ keyup_press = 0; Buzzer_toggle; } 
    Delay(300); 
	}
	
}
