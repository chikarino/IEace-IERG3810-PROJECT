#include "stm32f10x.h"
#include "IERG3810_Clock.h"
#include "IERG3810_USART.h"

void Delay(u32 count)
{
  u32 i;
  for (i = 0; i < count; i++) { __NOP(); }
}

int main(void)
{
  const char id[]   = "1155208116 on USART1\r\n";
  const char msg1[] = "IERG3810 Lab-2.4: CLOCK/USART libraries OK (delay)\r\n";
  const char msg2[] = "IERG3810 Lab-2.4: TXE-based print OK\r\n";

  clocktree_init();          /* SYS=72, APB1=36, APB2=72 */
  usart2_init(36, 9600);
  usart1_init(72, 9600);

  Delay(1000000); 
//Print
  usart_print(USART_PORT1, (char*)id);
  usart_print(USART_PORT2, (char*)msg1);
  usart_print_txe(USART_PORT2, (char*)msg2);

  while (1) {
    Delay(1000000);
    //usart_print_txe(USART_PORT2, "loop USART 2 TXE \r\n");
  }
}
