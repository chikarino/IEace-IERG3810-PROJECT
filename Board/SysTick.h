#ifndef SYSTICK_H
#define SYSTICK_H

/* Available heartbeat slot count (adjustable as needed) */
#define HB_SLOT_COUNT        16u
#define HB_TICKS_IMMEDIATE   0u

#include "stm32f10x.h"

/* Initialize SysTick, tick_interval_ms of 1 means 1ms increment */
void SysTick_Init(u32 tick_interval_ms);

/* Current accumulated milliseconds since boot */
u32 SysTick_GetTick(void);

/* Blocking delay (milliseconds) */
void SysTick_DelayMs(u32 ms);

void SysTick_IncTick(void);

/*
 * Initialize heartbeat module.
 * tick_interval_ms is the number of milliseconds per SysTick trigger (e.g., 10 means every 10 ms).
 */
void Heartbeat_Init(u32 tick_interval_ms);

/* Called by SysTick_Handler(), responsible for counting down each slot */
void Heartbeat_TickISR(void);

/* Start/restart specified slot, ticks is the "countdown unit" */
void Heartbeat_Start(u8 slot, u32 ticks);
void Heartbeat_Restart(u8 slot, u32 ticks);

/* Stop specified slot (set to 0) */
void Heartbeat_Stop(u8 slot);

/* Check if slot countdown has completed (equals 1) */
u8   Heartbeat_Expired(u8 slot);

/* Check if slot is still active (greater than 0) */
u8   Heartbeat_IsActive(u8 slot);

/* Convert milliseconds to ticks based on the initialized tick_interval_ms (rounding up) */
u32  Heartbeat_MsToTicks(u32 duration_ms);

/* Get the current tick interval (ms) */
u32  Heartbeat_GetTickIntervalMs(void);
#endif
