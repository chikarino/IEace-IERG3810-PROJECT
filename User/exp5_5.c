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
#define io_init() do{ \
    clocktree_init(); \
    IERG3810_LED_Init(); \
    IERG3810_KEY_Init(); \
    IERG3810_Buzzer_Init(); \
}while(0)
volatile u32 heartbeat[10] = {0};
void systick_init_10ms(void)
{
    //SYSTICK
    SysTick->CTRL = 0; //clear
                    
    SysTick->LOAD = 90000 - 1;  //10 ms → 0.01 s × 9 000 000 ticks/s = 90 000 ticks
                    //Refer to DDI-0337E
                    //Refer to 0337E page 8-10
    //CLKSOURCE = 0: STCLK (FLCK/8) 72 MHz / 8 = 9MHz
    //CLKSOURCE = 1: FLCK  72 MHz
    //set CLKSOURCE=0 is synchroized and bettern than CLKSOURCE=1
    //refer to Clock tree on Rm0008 page-93
    SysTick->CTRL &= ~(1U << 2); //CLKSOURCE=0
    SysTick->CTRL |= 1 << 1;    //Enable interrupt, counting down to 0 pends the SysTick handler.
    SysTick->CTRL |= 1 << 0;    //Enable countdown
                    //set internal clock, use interrupt, start count
 
}

int main (void)
{
    heartbeat[0]=10;    //after 100ms toggle DS0
    heartbeat[1]=17;    //after 170ms toggle DS1
    clocktree_init();
    io_init(); // LEDs, Keys, Buzzer init
    systick_init_10ms();

    while(1)
    {
        if (heartbeat[0]==1)
        {
            heartbeat[0]=10;    //reset counter 100ms
            GPIOB->ODR ^= 1<<5; // Toggle DS0 with read-modify-write
        }
        if (heartbeat[1]==1)
        {
            heartbeat[1]=17;    //reset counter 170ms
            GPIOE->ODR ^= 1<<5; // Toggle DS1 with read-modify-write
        }
    }
}
