#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_LED.h"
#include "IERG3810_KEY.h"
#include "IERG3810_Buzzer.h"
#include "Delay.h"
#include "IERG3810_EXTI.h"
#include "IERG3810_NVIC.h"
#include "SysTick.h"
#define DS1_toggle      do{ ds1_on ^= 1; if(ds1_on) DS1_ON; else DS1_OFF; }while(0)
#define DS0_toggle      do{ ds0_on ^= 1; if(ds0_on) DS0_ON; else DS0_OFF; }while(0)
#define Buzzer_toggle   do{ buz_on ^= 1; if(buz_on) BUZ_ON; else BUZ_OFF; }while(0)
u8 ds1_on = 0, buz_on = 0, ds0_on=0;

#define io_init() do{ \
    clocktree_init(); \
    IERG3810_LED_Init(); \
    IERG3810_KEY_Init(); \
    IERG3810_Buzzer_Init(); \
}while(0)
volatile u32 heartbeat[10]; //exp5.5
volatile u8 use_brr = 0;

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



void TIM3_IRQHandler(void) //exp5.3
{
    GPIOB->BRR= 1<<5; // LED ON
    GPIOB->BSRR= 1<<5; // LED OFF
    GPIOB->BRR= 1<<5; // LED ON
    GPIOB->BSRR= 1<<5; // LED OFF
    GPIOB->ODR ^= 1<<5; // Toggle LED
    GPIOB->ODR ^= 1<<5; // Toggle LED
    GPIOB->ODR ^= 1<<5; // Toggle LED
    GPIOB->ODR ^= 1<<5; // Toggle LED
    GPIOB->ODR &= ~(1<<5);  //use &=
    GPIOB->ODR |= 1<<5;   //use |=
    GPIOB->ODR &= ~(1<<5);  //use &=
    GPIOB->ODR |= 1<<5;   //use |=
    TIM3->SR &= ~(1<<0); // Clear UIF flag, RM0008 v21 P410
    TIM3->SR &= ~(1<<0); // Clear UIF flag, RM0008 v21 P410

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
    tim3_init(4999, 7199); // DS0 toggle every 0.5s
    //tim4_init(1249, 7199); // DS1 toggle every 0.25s
    tim4_init(4999, 7199); // DS1 toggle every 0.5s

    DS0_OFF;
    DS1_OFF;
    while(1){
        ;
    }
}
