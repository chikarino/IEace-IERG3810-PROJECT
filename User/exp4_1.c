#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_LED.h"
#include "IERG3810_KEY.h"
#include "IERG3810_Buzzer.h"
#include "Delay.h"
#include "IERG3810_EXTI.h"
#include "IERG3810_NVIC.h"
#define DS1_toggle      do{ ds1_on ^= 1; if(ds1_on) DS1_ON; else DS1_OFF; }while(0)
#define Buzzer_toggle   do{ buz_on ^= 1; if(buz_on) BUZ_ON; else BUZ_OFF; }while(0)

//EXTI handler for KEY2
void EXTI2_IRQHandler(void){
  u8 i;
  for (i=0; i<10; i++){
    DS0_ON;
    Delay(1000000);
    DS0_OFF;
    Delay(1000000);
  }
  EXTI->PR = 1<<2; //clear pending bit
}

int main(void){
  /*IO init*/
  IERG3810_LED_Init();
  IERG3810_KEY_Init();
  IERG3810_Buzzer_Init();
  /*********/
  clocktree_init();
  nvic_setPriorityGroup(5);
  key2_extiInit(0x65); //Exp4.1 Init Key2 as EXTI
  DS0_OFF;

  while(1){
    DS1_ON;
    Delay(500000);
    DS1_OFF;
    Delay(500000);
  }
}
