// 目標：HSE(8 MHz) → PLL×9 → SYSCLK=72 MHz；AHB=72 MHz；APB1=36 MHz；APB2=72 MHz
// 參考：RM0008 v21 — RCC->CR, RCC->CFGR, FLASH->ACR
#include "IERG3810_USART.h"
extern void Delay(u32 count);

// 目標：初始化 USART2 為指定鮑率（例如 36 MHz/9600 bps → BRR=0x0EA6），僅啟用發送（TE）。
// 參考：RM0008 v21 — USART->BRR、USART->CR1、GPIOx_CRL、RCC APB1/2 ENR & RSTR
void usart2_init(u32 pclk1, u32 baud)
{
  // -------- 計算 BRR (USART_BRR) --------
  // USARTDIV = PCLK1 / (16 * baud)
  // BRR = mantissa<<4 | fraction，其中 fraction = (USARTDIV 的小數部分) × 16
  float temp;                  // 暫存 USARTDIV（浮點）
  u16   mantissa;              // 整數部分（將放入 BRR[15:4]）
  u16   fraction;              // 小數×16（將放入 BRR[3:0]）

  // 注意：此函式的 pclk1 以「MHz」傳入，所以先 ×1,000,000 變 Hz
  temp = (float)(pclk1 * 1000000) / (baud * 16);

  mantissa = (u16)temp;                       // 取整數部分
  fraction = (u16)((temp - mantissa) * 16);   // 取小數部分×16（截斷法）
  mantissa <<= 4;                             // 移到 BRR[15:4]
  mantissa += fraction & 0xF;                 // 加上 BRR[3:0]（保險地遮 4 位）

  // -------- 開外設時鐘 --------
  RCC->APB2ENR |= 1 << 2;                     // GPIOAEN（APB2ENR bit2）
  RCC->APB1ENR |= 1 << 17;                    // USART2EN（APB1ENR bit17）

  // -------- 配置 PA2/PA3 腳位功能 --------
  // CRL 控制 pin0~pin7；pin2 對應位[11:8]，pin3 對應位[15:12]。每 pin 4 位：CNF[1:0] + MODE[1:0]
  GPIOA->CRL &= 0xFFFF00FF;                   // 先清掉 pin2、pin3 共 8 位

  // 0x00008B00 →
  //   pin2: 0xB (1011b) = CNF2=10(復用推挽 AF PP) + MODE2=11(50 MHz)
  //   pin3: 0x8 (1000b) = CNF3=10(輸入上/下拉)   + MODE3=00(輸入)
  // 若用 PU/PD 模式，請再以 ODR3 決定上/下拉方向：上拉→GPIOA->BSRR=(1u<<3)；下拉→GPIOA->BRR=(1u<<3)
  // 只做發送時對 TX 無影響；但日後要接收時建議改成「輸入浮空 0x4」或顯式上拉。
  GPIOA->CRL |= 0x00008B00;

  // -------- 軟重置 USART2（確保乾淨狀態）--------
  RCC->APB1RSTR |=  1 << 17;                  // 置 1 進入 reset
  RCC->APB1RSTR &= ~(1 << 17);                // 清 0 結束 reset

  // -------- 設定鮑率與啟用發送 --------
  USART2->BRR = mantissa;                     // 例如 pclk1=36、baud=9600 → 0x0EA6
  USART2->CR1 = 0x2008;                       // UE(bit13)=1 使能 USART；TE(bit3)=1 啟用發送（8N1 預設）
  // 若要啟用接收：USART2->CR1 |= (1u<<2); // RE(bit2)=1
}

void usart1_init(u32 pclk2, u32 baud)
{
  // -------- 計算 BRR：與 usart2_init 同風格 --------
  float temp;
  u16   mantissa;
  u16   fraction;

  temp     = (float)(pclk2 * 1000000u) / (baud * 16u);  // pclk2 以 MHz 傳入
  mantissa = (u16)temp;
  fraction = (u16)((temp - mantissa) * 16.0f);
  mantissa = (mantissa << 4) + (fraction & 0xF);        // 合併成 BRR

  // -------- 開時鐘（USART1 在 APB2）--------
  RCC->APB2ENR |= (1u << 2);    // GPIOAEN
  RCC->APB2ENR |= (1u << 14);   // USART1EN
  // 可選：RCC->APB2ENR |= (1u << 0); // AFIOEN（若未用重映射，可不開）

  // -------- 配腳：PA9=TX、PA10=RX --------
  // CRH 控 pin8~15；pin9 對應 [7:4]，pin10 對應 [11:8]
  GPIOA->CRH &= ~((0xFu << 4) | (0xFu << 8));   // 先清掉兩個 nibble
  GPIOA->CRH |= (0xB  << 4);                    // PA9: 0xB = AF PP, 50 MHz
  GPIOA->CRH |= (0x4  << 8);                    // PA10: 0x4 = Input floating
  // 若想用上拉輸入：把上行 0x4 改 0x8，並加 GPIOA->BSRR = (1u<<10);

  // -------- 軟重置 USART1（乾淨狀態）--------
  RCC->APB2RSTR |=  (1u << 14);
  RCC->APB2RSTR &= ~(1u << 14);

  // -------- 寫 BRR 與啟用 UE/TE --------
  USART1->BRR = mantissa;       // 72MHz/9600 → 0x1D4C
  USART1->CR1 = 0x2008;         // UE=1、TE=1（僅發送；8N1 為預設）
  // 若要同時接收：USART1->CR1 |= (1u<<2); // RE=1
}


void usart_print(u8 USARTport, char *st)
{
  u8 i = 0;                      
  while (st[i] != 0x00)              // 以 0x00 作字串結尾
  {
    if (USARTport == 1) USART1->DR = (u16)st[i];
    if (USARTport == 2) USART2->DR = (u16)st[i];

    Delay(50000);                    // 固定延遲，避免覆蓋資料暫存器
    if (i == 255) break;             // 最大長度 256
    i++;
  }
}

void usart_print_txe(u8 USARTport, char *st)
{
  u8 i = 0; 
  while (st[i] != 0x00)
  {
    if (USARTport == 1)
    {
      while ((USART1->SR & (1u<<7)) == 0) { }   // wait for USART1->SR  TXE(bit7)=1
      USART1->DR = (u16)st[i];
    }
    if (USARTport == 2)
    {
      while ((USART2->SR & (1u<<7)) == 0) { }   // wait for USART2->SR  TXE(bit7)=1
      USART2->DR = (u16)st[i];
    }

    if (i == 255) break;                         // 256 max
    i++;
  }

}
