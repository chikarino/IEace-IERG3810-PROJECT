#include "SysTick.h"

static volatile u32 hb_slots[HB_SLOT_COUNT];
static u32          hb_tick_interval_ms = 10u;   /* Default 10ms */
static u32          hb_accumulated_ms   = 0u;    /* Accumulated 1ms ticks */


static volatile u32 systick_ms = 0;  /* Millisecond count accumulated by interrupt */
void SysTick_Init(u32 tick_interval_ms)
{
  u32 reload;

  /* tick_interval_ms cannot be 0 */
  if (tick_interval_ms == 0u) return;

  /* 72MHz / 1000 = 72,000 → 1ms，注意 SysTick reload 有 24bit 限制 */
  reload = (SystemCoreClock / 1000u) * tick_interval_ms;
  if (reload > 0x00FFFFFFu) return;  /* Exceeds range, abandon setting */

  systick_ms = 0;

  SysTick->CTRL = 0;                 /* Disable SysTick first */
  SysTick->LOAD = reload - 1u;       /* Reload value */
  SysTick->VAL  = 0;                 /* Clear current count */
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                  SysTick_CTRL_TICKINT_Msk   |
                  SysTick_CTRL_ENABLE_Msk;   /* Core clock, enable interrupt and start */
}

u32 SysTick_GetTick(void)
{
  return systick_ms;
}

void SysTick_DelayMs(u32 ms)
{
  u32 start;

  start = SysTick_GetTick();
  while ((SysTick_GetTick() - start) < ms) {
    /* Busy wait*/
  }
}

/* This function is called in stm32f10x_it.c's SysTick_Handler() */
void SysTick_IncTick(void)
{
  systick_ms++;
}


/* ────────── Heartbeat Area ────────── */

void Heartbeat_Init(u32 tick_interval_ms)
{
    u8 i;

    if (tick_interval_ms == 0u) {
        tick_interval_ms = 1u;
    }

    hb_tick_interval_ms = tick_interval_ms;
    hb_accumulated_ms   = 0u;

    for (i = 0u; i < HB_SLOT_COUNT; i++) {
        hb_slots[i] = 0u;
    }
}

void Heartbeat_TickISR(void)
{
    u8 i;

    /* Each time SysTick (1ms) enters, accumulate milliseconds */
    hb_accumulated_ms++;

    /* When accumulated time reaches or exceeds the configured heartbeat interval (default 10ms), perform countdown */
    while (hb_accumulated_ms >= hb_tick_interval_ms) {
        hb_accumulated_ms -= hb_tick_interval_ms;

        for (i = 0u; i < HB_SLOT_COUNT; i++) {
            u32 val = hb_slots[i];
            if (val > 1u) {
                hb_slots[i] = val - 1u;
            }
        }
    }
}

void Heartbeat_Start(u8 slot, u32 ticks)
{
    if (slot >= HB_SLOT_COUNT) {
        return;
    }

    if (ticks == 0u) {
        hb_slots[slot] = 1u;          /* Immediate trigger (will expire next round) */
    } else {
        hb_slots[slot] = ticks + 1u;  /* +1 so countdown reaches 1 when zero */
    }
}

void Heartbeat_Restart(u8 slot, u32 ticks)
{
    Heartbeat_Start(slot, ticks);
}

void Heartbeat_Stop(u8 slot)
{
    if (slot >= HB_SLOT_COUNT) {
        return;
    }
    hb_slots[slot] = 0u;
}

u8 Heartbeat_Expired(u8 slot)
{
    if (slot >= HB_SLOT_COUNT) {
        return 0u;
    }
    return (hb_slots[slot] == 1u) ? 1u : 0u;
}

u8 Heartbeat_IsActive(u8 slot)
{
    if (slot >= HB_SLOT_COUNT) {
        return 0u;
    }
    return (hb_slots[slot] != 0u) ? 1u : 0u;
}

u32 Heartbeat_MsToTicks(u32 duration_ms)
{
    u32 interval = hb_tick_interval_ms; //10ms

    if (duration_ms == 0u) {
        return 0u;
    }

    return (duration_ms + interval - 1u) / interval;
}

u32 Heartbeat_GetTickIntervalMs(void)
{
    return hb_tick_interval_ms;
}
