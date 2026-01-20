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
volatile u32 heartbeat[10]; //exp5.5
/*Simple test ISR/MAIN flag*/
#define TEST_IN_ISR   0
#define TEST_IN_MAIN   1

//-- Exp-5.1 Given
void tim3_init (u16 arr, u16 psc)
{
    //TIM3 IRQ#29
    RCC->APB1ENR |= 1<<1; // set TIM3EN bit-1 RM0008 v21 p115
    TIM3->ARR = arr;      //Auto-reload register RM0008 v21 P356
    TIM3->PSC = psc;      //Prescaler RM0008 v21 P356
    TIM3->DIER |= 1<<0; //UIE: Update interrupt enable RM0008 v21 P363
    TIM3->CR1 |= 1<<0; //CEN: Counter enable RM0008 v21 P363
    NVIC->IP[29] = 0x45; //set priority of this interrupt
    NVIC->ISER[0] |= 1<<29; //enable TIM3 IRQ#29
}

//--exp5.2
void tim4_init (u16 arr, u16 psc)
{
    //TIM4 IRQ#30
    RCC->APB1ENR |= 1<<2; // set TIM4EN bit-2 RM0008 v21 p115
    TIM4->ARR = arr;      //Auto-reload register RM0008 v21 P356
    TIM4->PSC = psc;      //Prescaler RM0008 v21 P356
    TIM4->DIER |= 1<<0; //UIE: Update interrupt enable RM0008 v21 P363
    TIM4->CR1 |= 1<<0; //CEN: Counter enable RM0008 v21 P363
    NVIC->IP[30] = 0x25; //set priority of this interrupt
    NVIC->ISER[0] |= 1<<30; //enable TIM4 IRQ#30
}

void ds0_turnOff(void)//exp5.4 given
{
    GPIOB->BSRR = 1<<5;
}
void ds0_turnOff2(void)//exp5.4 given
{
    ds0_turnOff();
}

//---- EXTI handlers must be placed in main.c ----
void TIM3_IRQHandler(void) //exp5.4 given
{
#if TEST_IN_ISR
        DS0_ON;
        DS0_OFF;
        DS0_ON;
        ds0_turnOff();
        DS0_ON;
        ds0_turnOff2();
        DS0_ON;
#endif
   // TIM3->SR &= ~(1<<0); // NO CLEAR
}
void TIM4_IRQHandler(void) //DS1 exp5.2
{
    if (TIM4->SR & 1<<0){ //UIF: Update interrupt flag RM0008 v21 P410
        GPIOE->ODR ^= 1<<5; // Toggle DS1 with read-modify-write
    }
    TIM4->SR &= ~(1<<0); // Clear UIF flag, RM0008 v21 P410
}   


int main (void) //-- Exp-5.1 Given
{ 
    clocktree_init();
    io_init(); // LEDs, Keys, Buzzer init
    nvic_setPriorityGroup(5); 
    tim4_init(1249, 7199); // DS1 toggle every 0.25s
#if TEST_IN_ISR
    tim3_init(4999, 7199); // DS0 toggle every 0.5s 
#endif
    DS0_OFF;
    DS1_OFF;
    while(1){
#if TEST_IN_MAIN
        DS0_ON;
        DS0_OFF;
        DS0_ON;
        ds0_turnOff();
        DS0_ON;
        ds0_turnOff2();
        DS0_ON;
#endif
    }
}

