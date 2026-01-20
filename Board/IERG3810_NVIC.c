#include "IERG3810_NVIC.h"

void nvic_setPriorityGroup(u8 priGroup){
  u32 tmp1, tmp2;
  tmp2 = priGroup & 0x00000007 << 8;//only 3 bits are used
  tmp1 = SCB->AIRCR &= 0x0000F8FF; //--ARMDI0337 p.8-22 //read old register content
  tmp1 |= 0x05FA0000; //write VECTKEY //must read ARMDI0337 p.8-22
  SCB->AIRCR = tmp1 | tmp2; //write new priority group
}
