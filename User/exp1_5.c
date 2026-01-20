#include "stm32f10x.h"
#include "IERG3810_LED.h"
#include "IERG3810_KEY.h"
#include "IERG3810_Buzzer.h"

u8 key1_press = 0;
u8 keyup_press = 0;
u8 ds1_on = 0, buz_on = 0;

void Delay(u32 count){
	u32 i;
	for (i=0; i<count; i++);
}

#define DS1_toggle      do{ ds1_on ^= 1; if(ds1_on) DS1_ON; else DS1_OFF; }while(0)
#define Buzzer_toggle   do{ buz_on ^= 1; if(buz_on) BUZ_ON; else BUZ_OFF; }while(0)



int main(void){

  IERG3810_LED_Init();
  IERG3810_KEY_Init();
  IERG3810_Buzzer_Init();

	
	while (1){
    if (KEY2_DOWN) DS0_ON; else DS0_OFF;
		
    if (IERG3810_KEY1_ReadEdge()){ key1_press = 0; DS1_toggle; } 

    if (IERG3810_KEYUP_ReadEdge()){ keyup_press = 0; Buzzer_toggle; } 
    Delay(300); 
	}
	
}
