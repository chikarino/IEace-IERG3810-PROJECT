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

volatile u32 heartbeat[10] = {0};

#define TIM3_PWM_ARR              9999
#define TIM3_PWM_PSC              47      /* 72 MHz / (47 + 1) / (9999 + 1) = 150 Hz */
#define COLOR_STEP_PERIOD_10MS    40      //e.g. 40 * 10 = 400ms
#define COLOR_HEARTBEAT_SLOT      0u

#define COLOR_MODE                1        //The two mode according to manual // 1 → mode1, 2 → mode2

#define RGB_FULL_DUTY             5000    
#define RGB_OFF_DUTY              0

typedef struct {
    u16 r;
    u16 g;
    u16 b;
} RGBColor;

 RGBColor color_seq_mode1[] = {
    {RGB_FULL_DUTY, RGB_OFF_DUTY, RGB_OFF_DUTY},                     /* RED */
    {RGB_FULL_DUTY, RGB_FULL_DUTY, RGB_OFF_DUTY},                    /* YELLOW */
    {RGB_OFF_DUTY,  RGB_FULL_DUTY, RGB_OFF_DUTY},                    /* GREEN */
    {RGB_OFF_DUTY,  RGB_FULL_DUTY, RGB_FULL_DUTY},                   /* CYAN */
    {RGB_OFF_DUTY,  RGB_OFF_DUTY, RGB_FULL_DUTY},                    /* BLUE */
    {RGB_FULL_DUTY, RGB_OFF_DUTY, RGB_FULL_DUTY}                     /* MAGENTA */
};

 RGBColor color_seq_mode2[] = {
    {RGB_FULL_DUTY, RGB_OFF_DUTY, RGB_OFF_DUTY},                     /* RED */
    {RGB_OFF_DUTY,  RGB_OFF_DUTY, RGB_OFF_DUTY},                     /* DARK */
    {RGB_FULL_DUTY, RGB_FULL_DUTY, RGB_OFF_DUTY},                    /* YELLOW */
    {RGB_OFF_DUTY,  RGB_OFF_DUTY, RGB_OFF_DUTY},                     /* DARK */
    {RGB_OFF_DUTY,  RGB_FULL_DUTY, RGB_OFF_DUTY},                    /* GREEN */
    {RGB_OFF_DUTY,  RGB_OFF_DUTY, RGB_OFF_DUTY},                     /* DARK */
    {RGB_OFF_DUTY,  RGB_FULL_DUTY, RGB_FULL_DUTY},                   /* CYAN */
    {RGB_OFF_DUTY,  RGB_OFF_DUTY, RGB_OFF_DUTY},                     /* DARK */
    {RGB_OFF_DUTY,  RGB_OFF_DUTY, RGB_FULL_DUTY},                    /* BLUE */
    {RGB_OFF_DUTY,  RGB_OFF_DUTY, RGB_OFF_DUTY},                     /* DARK */
    {RGB_FULL_DUTY, RGB_OFF_DUTY, RGB_FULL_DUTY},                    /* MAGENTA */
    {RGB_OFF_DUTY,  RGB_OFF_DUTY, RGB_OFF_DUTY}                      /* DARK */
};

#if COLOR_MODE == 1
    #define ACTIVE_COLOR_SEQUENCE          color_seq_mode1
    #define ACTIVE_COLOR_SEQUENCE_LENGTH   (sizeof(color_seq_mode1) / sizeof(color_seq_mode1[0]))
#elif COLOR_MODE == 2
    #define ACTIVE_COLOR_SEQUENCE          color_seq_mode2
    #define ACTIVE_COLOR_SEQUENCE_LENGTH   (sizeof(color_seq_mode2) / sizeof(color_seq_mode2[0]))
#else
    
#endif

void heartbeat_start(u8 slot, u32 ticks10ms)
{
    if (ticks10ms == 0U) {
        heartbeat[slot] = 1U;
    } else {
        heartbeat[slot] = ticks10ms + 1U;
    }
}

u8 heartbeat_expired(u8 slot)
{
    return (heartbeat[slot] == 1);
}


void tim3_init_rgb_pwm(u16 arr, u16 psc)
{
    RCC->APB1ENR |= (1 << 1);   /* TIM3 clock */
    RCC->APB2ENR |= (1 << 0);   /* AFIO clock */
    RCC->APB2ENR |= (1 << 4);   /* GPIOC clock */
    
    /* TIM3 full remap: CH1/CH2/CH3 -> PC6/PC7/PC8 */
    AFIO->MAPR &= ~(3 << 10);
    AFIO->MAPR |=  (3 << 10);   /* 0b11 -> full remap */

    /* PC6, PC7: AF Push-Pull 50 MHz */
    GPIOC->CRL &= ~((0xFu << 24) | (0xFu << 28));
    GPIOC->CRL |=  ((0xBu << 24) | (0xBu << 28));

    /* PC8: AF Push-Pull 50 MHz */
    GPIOC->CRH &= ~(0xF << 0);
    GPIOC->CRH |=  (0xB << 0);

    TIM3->ARR = arr;
    TIM3->PSC = psc;
    TIM3->CR1 = 0x0000;
    TIM3->CR1 |= (1 << 7);      /* set ARPE (auto-reload preload enable) */
    TIM3->EGR |= 1;             /* UG: update registers  (initialize)*/

    /* Channel 1 (PC6) */
    TIM3->CCMR1 &= ~(0x7 << 4);
    TIM3->CCMR1 |=  (0x7 << 4); /* PWM mode 2 */
    TIM3->CCMR1 |=  (1 << 3);   /* OC1PE preload*/
    TIM3->CCR1 = RGB_OFF_DUTY;

    /* Channel 2 (PC7) */
    TIM3->CCMR1 &= ~(0x7 << 12);
    TIM3->CCMR1 |=  (0x7 << 12);/* PWM mode 2 */
    TIM3->CCMR1 |=  (1 << 11);  /* OC2PE preload*/
    TIM3->CCR2 = RGB_OFF_DUTY;

    /* Channel 3 (PC8) */
    TIM3->CCMR2 &= ~(0x7 << 4);
    TIM3->CCMR2 |=  (0x7 << 4); /* PWM mode 2 */
    TIM3->CCMR2 |=  (1 << 3);   /* OC3PE preload*/
    TIM3->CCR3 = RGB_OFF_DUTY;

    TIM3->CCER |= (1 << 0) | (1 << 4) | (1 << 8); /* Enable CH1/2/3 */
    TIM3->CR1   |= 0x01;          // start TIM3 counter                                // RM0008 v21 p404
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


void rgb_apply_color(const RGBColor *color)
{
    TIM3->CCR1 = color->r;
    TIM3->CCR2 = color->g;
    TIM3->CCR3 = color->b;
}


int main(void)
{
    u8 current_color = 0;

    io_init();
    tim3_init_rgb_pwm(TIM3_PWM_ARR, TIM3_PWM_PSC);
    tim4_init(1999, 7199);               /* DS1  5 Hz  */
    systick_init_10ms();

    rgb_apply_color(&ACTIVE_COLOR_SEQUENCE[current_color]);
    heartbeat_start(COLOR_HEARTBEAT_SLOT, COLOR_STEP_PERIOD_10MS);

    while (1) {
        if (heartbeat_expired(COLOR_HEARTBEAT_SLOT)) //When heartbeat slot is counted to 1 
         {
            current_color++;    //Next color
            if (current_color >= ACTIVE_COLOR_SEQUENCE_LENGTH) {
                current_color = 0; //Wrap to first color
            }
            rgb_apply_color(&ACTIVE_COLOR_SEQUENCE[current_color]);
            heartbeat_start(COLOR_HEARTBEAT_SLOT, COLOR_STEP_PERIOD_10MS); //Reschedule counter
        }
    }
}


void TIM4_IRQHandler(void)
{
    if (TIM4->SR & (1U << 0)) {
        GPIOE->ODR ^= (1U << 5);          /* DS1 toggle */
        TIM4->SR &= ~(1U << 0);
    }
}
