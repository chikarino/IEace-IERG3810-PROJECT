#include "JoyPad.h"

// Board v2 Pin Mapping:
// Latch: PC8  (Output)
// Data:  PC9  (Input)
// Clock: PC12 (Output)

// Data is active low (Low = Pressed)
// We return 1 if pressed

void JoyPad_Init(void)
{
    // Enable GPIOC Clock
    RCC->APB2ENR |= (1 << 4); // IOPCEN

    // Configure PC8 (Latch) as Output Push-Pull, 50MHz
    // PC8 is in CRH (bits 0-3)
    GPIOC->CRH &= 0xFFFFFFF0; 
    GPIOC->CRH |= 0x00000003; 

    // Configure PC12 (Clock) as Output Push-Pull, 50MHz
    // PC12 is in CRH (bits 16-19)
    GPIOC->CRH &= 0xFFF0FFFF; 
    GPIOC->CRH |= 0x00030000; 

    // Configure PC9 (Data) as Input with Pull-up
    // PC9 is in CRH (bits 4-7)
    // CNF=10 (Input with pull-up/down), MODE=00 (Input) -> 0x8
    GPIOC->CRH &= 0xFFFFFF0F; 
    GPIOC->CRH |= 0x00000080; 
    GPIOC->ODR |= (1 << 9);   // Set ODR bit 9 for Pull-up

    // Initial states
    GPIOC->BSRR = (1 << 8);   // Latch High
    GPIOC->BRR  = (1 << 8);   // Latch Low
}

u8 JoyPad_Read(void)
{
    u8 i;
    u8 data = 0;

    // 1. Latch Pulse (High -> Low) to capture button states
    // Using ODR |= and &=~ creates a Read-Modify-Write sequence
    // which naturally provides enough delay for the pulse width.
    GPIOC->ODR |= (1 << 8);   // Latch High
    GPIOC->ODR &= ~(1 << 8);  // Latch Low

    // 2. Read 8 buttons sequentially
    for (i = 0; i < 8; i++) {
        // Read current bit from IDR
        if ((GPIOC->IDR & (1 << 9)) == 0) { // Active low
            data |= (1 << i);
        }
        // Pulse Clock (High -> Low) to shift to next bit
        GPIOC->ODR |= (1 << 12);   // Clock High
        GPIOC->ODR &= ~(1 << 12);  // Clock Low
    }

    return data;
}
