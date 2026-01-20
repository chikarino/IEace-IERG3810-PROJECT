#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_LED.h"
#include "IERG3810_KEY.h"
#include "IERG3810_Buzzer.h"
#include "Delay.h"
#include "IERG3810_EXTI.h"
#include "IERG3810_NVIC.h"
#include "IERG3810_USART.h"

#define DS1_toggle      do{ ds1_on ^= 1; if(ds1_on) DS1_ON; else DS1_OFF; }while(0)
#define Buzzer_toggle   do{ buz_on ^= 1; if(buz_on) BUZ_ON; else BUZ_OFF; }while(0)
#define io_init() do{ \
    clocktree_init(); \
    IERG3810_LED_Init(); \
    IERG3810_KEY_Init(); \
    IERG3810_Buzzer_Init(); \
}while(0)
volatile u32 heartbeat[10] = {0}; //exp5.5
#define LED0_PWM_VAL TIM3->CCR2 //-- given for exp5.6
#define LED0_PWM_MAX    9999       // Maximum duty cycle corresponding to ARR
#define LED0_PWM_STEP   80        // Step for every adjustment
#define LED0_UPDATE_TICKS   2      //2*10 - -10 = 10ms
u16 led0pwmval = 0;          //-- given for exp5.6
u8 dir = 1;                  //-- given for exp5.6

#define TEST_IN_DELAY   0
#define TEST_IN_SYSTICK  1

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

void TIM4_IRQHandler(void) //DS1 exp5.2
{
    if (TIM4->SR & 1<<0){ //UIF: Update interrupt flag RM0008 v21 P410
        GPIOE->ODR ^= 1<<5; // Toggle DS1 with read-modify-write
    }
    TIM4->SR &= ~(1<<0); // Clear UIF flag, RM0008 v21 P410
}  

void systick_init_10ms(void)
{
    //SYSTICK
    SysTick->CTRL = 0; //clear
                    
    SysTick->LOAD = 90000 - 1;  //10 ms → 0.01 s × 9 000 000 ticks/s = 90 000 ticks
                    //Refer to DDI-0337E
                    //Refer to 0337E page 8-10
    //CLKSOURCE = 0: STCLK (FLCK/8) 72 MHz / 8 = 9 MHz
    //CLKSOURCE = 1: FLCK  72 MHz
    //set CLKSOURCE=0 is synchroized and bettern than CLKSOURCE=1
    //refer to Clock tree on Rm0008 page-93
    SysTick->CTRL &= ~(1U << 2); //CLKSOURCE=0
    SysTick->CTRL |= 1 << 1;    //Enable interrupt, counting down to 0 pends the SysTick handler.
    SysTick->CTRL |= 1 << 0;    //Enable countdown
                    //set internal clock, use interrupt, start count
 
}

//-- tim3_init_pwm() --- exp5.6 given
void tim3_init_pwm(u16 arr, u16 psc)
{
    RCC->APB2ENR |= 1<<3;         // IOPBEN                     // RM0008 v21 p112
    GPIOB->CRL &= 0xFF0FFFFF;     // clear PB5 mode bits for reconfiguration           // RM0008 v21 p171
    GPIOB->CRL |= 0x00B00000;     // set PB5 as Alternate Function Push-Pull (50 MHz)  // RM0008 v21 p171
    RCC->APB2ENR |= 1<<0;         // enable AFIO clock                                 // RM0008 v21 p112
    AFIO->MAPR &= 0xFFFFF3FF;     // clear TIM3 remap bits                             // RM0008 v21 p184
    AFIO->MAPR |= 1<<11;          // partially remap TIM3 CH2 to PB5                   // RM0008 v21 p184
    RCC->APB1ENR |= 1<<1;         // enable TIM3 clock                                 // RM0008 v21 p115
    TIM3->ARR = arr;              // set auto-reload value (PWM period)                // RM0008 v21 p419
    TIM3->PSC = psc;              // set prescaler value                               // RM0008 v21 p418
    TIM3->CCMR1 |= 7<<12;         // configure CH2 as PWM mode 1                       // RM0008 v21 p413
    TIM3->CCMR1 |= 1<<11;         // enable CH2 preload for CCR2                       // RM0008 v21 p413
    TIM3->CCER  |= 1<<4;          // enable output on channel 2                        // RM0008 v21 p417
    TIM3->CR1    = 0x0080;        // set ARPE (auto-reload preload enable)             // RM0008 v21 p404
    TIM3->CR1   |= 0x01;          // start TIM3 counter                                // RM0008 v21 p404
}

int main (void) //-- Exp-5.1 Given
{ 
    clocktree_init();
    io_init(); // LEDs, Keys, Buzzer init
    tim3_init_pwm(9999, 47);  //150Hz PWM
    tim4_init(1999, 7199); //5Hz
    systick_init_10ms();
    heartbeat[0] = LED0_UPDATE_TICKS; 
    LED0_PWM_VAL = led0pwmval;         // initialise CCR2

    while (1)
    {
        #if TEST_IN_SYSTICK
        if (heartbeat[0] == 1)
        {
            heartbeat[0] = LED0_UPDATE_TICKS;   // Reschedule

            if (dir) //increment to max
            {
                if (led0pwmval + LED0_PWM_STEP >= LED0_PWM_MAX)
                {
                    led0pwmval = LED0_PWM_MAX;
                    dir = 0;
                }
                else
                {
                    led0pwmval += LED0_PWM_STEP;
                }
            }
            else //decrement to 0
            {
                if (led0pwmval <= LED0_PWM_STEP)
                {
                    led0pwmval = 0;
                    dir = 1;
                }
                else
                {
                    led0pwmval -= LED0_PWM_STEP;
                }
            }

            LED0_PWM_VAL = led0pwmval; //put to register to change duty -> brightness
        }
            #endif
                #if TEST_IN_DELAY
    delay(1500);
    if (dir) led0pwmval++;
    else led0pwmval--;
    if (led0pwmval >5000) dir=0;
    if (led0pwmval == 0) dir=1;
    LED0_PWM_VAL = led0pwmval; //put to register to change duty -> brightness

    #endif
    }


}
