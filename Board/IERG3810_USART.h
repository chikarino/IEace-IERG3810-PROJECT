#define IERG3810_USART_H
#include "stm32f10x.h"

#define USART_PORT1 ((u8)1)

#define USART_PORT2 ((u8)2)


void usart1_init(u32 pclk2_mhz, u32 baud);
void usart2_init(u32 pclk1_mhz, u32 baud);

void usart_print(u8 USARTport, char *st);      // fixed delay
void usart_print_txe(u8 USARTport, char *st);  //  TXE

