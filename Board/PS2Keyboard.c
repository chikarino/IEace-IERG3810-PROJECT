#include "PS2Keyboard.h"
#include "IERG3810_EXTI.h"
#include "IERG3810_NVIC.h"

#define PS2_EVENT_FIFO_SIZE 16  // FIFO size for parsed PS/2 key events
#define PS2_RAW_FIFO_SIZE   32  // FIFO size for raw PS/2 bytes read from clock edges

static volatile u8  ps2BitCount = 0;  // Count of bits received for current PS/2 frame
static volatile u16 ps2ShiftReg = 0;  // Shift register collecting incoming PS/2 bits

static volatile u8 rawFifo[PS2_RAW_FIFO_SIZE];  // Circular buffer of raw PS/2 bytes
static volatile u8 rawHead = 0;                 // Write index for raw FIFO
static volatile u8 rawTail = 0;                 // Read index for raw FIFO

typedef struct {
    PS2KeyEvent buffer[PS2_EVENT_FIFO_SIZE];
    volatile u8 head;
    volatile u8 tail;
} PS2EventFIFO;

static PS2EventFIFO g_evtFifo;  // Global FIFO holding parsed key events

static u8 ps2BreakPending    = 0;  // Flag indicating next byte is a break code
static u8 ps2ExtendedPending = 0;  // Flag indicating next byte is an extended code

static void raw_push(u8 data) // Push a byte into the raw circular FIFO if space exists.
{
    // This is called from ISR context so it must be simple and non-blocking.
    u8 next = (rawHead + 1u) % PS2_RAW_FIFO_SIZE;
    if (next != rawTail) {
        rawFifo[rawHead] = data;
        rawHead = next;
    }
}

static int raw_pop(u8 *data) // Pop a byte from the raw FIFO if available and return success status.
{
    // Used by PS2_Update() to consume raw bytes for higher-level parsing.
    if (rawHead == rawTail) return 0;
    *data = rawFifo[rawTail];
    rawTail = (rawTail + 1u) % PS2_RAW_FIFO_SIZE;
    return 1;
}

static void evt_push(PS2EventFIFO *fifo, const PS2KeyEvent *evt) // Push a parsed key event into the event FIFO if space exists.
{
    // Maintains a small circular buffer for deliverable key events.
    u8 next = (fifo->head + 1u) % PS2_EVENT_FIFO_SIZE;
    if (next != fifo->tail) {
        fifo->buffer[fifo->head] = *evt;
        fifo->head = next;
    }
}

static int evt_pop(PS2EventFIFO *fifo, PS2KeyEvent *evt) // Pop a parsed key event from the event FIFO and return success status.
{
    // This is the final consumer path for decoded PS/2 events.
    if (fifo->head == fifo->tail) return 0;
    *evt = fifo->buffer[fifo->tail];
    fifo->tail = (fifo->tail + 1u) % PS2_EVENT_FIFO_SIZE;
    return 1;
}

void PS2_Init(void) // Reset PS/2 state machine, FIFOs, and EXTI configuration.
{
    // Configures the PS/2 clock EXTI so OnClockFallingEdge() is called on clock edges.
    ps2BitCount = 0;
    ps2ShiftReg = 0;
    rawHead = rawTail = 0;
    g_evtFifo.head = 0;
    g_evtFifo.tail = 0;
    ps2BreakPending    = 0;
    ps2ExtendedPending = 0;
    ps2key_extiInit(0x10);
}

void PS2_OnClockFallingEdge(void) // Handle PS/2 clock falling edge and assemble bits into bytes.
{
    u32 dataBit;
    u8  count;
    u8  stopBit;

    if ((EXTI->PR & (1 << 11)) == 0) return;
    // Ensure interrupt pending bit is set for the PS/2 clock EXTI line; avoid unrelated triggers.
    dataBit = (GPIOC->IDR >> 10) & 0x1;
    // Read the data line at the moment of falling clock to sample the bit.
    count   = ps2BitCount;
    if (count == 0) { // Check for the start bit; valid PS/2 start bit must be 0.
        if (dataBit != 0) {
            ps2BitCount = 0;
            ps2ShiftReg = 0;
            EXTI->PR = (1 << 11);
            return;
        }
        ps2ShiftReg = 0;
    }
    if (count < 11) { // Shift in the sampled bit at LSB position for the current frame.
        ps2ShiftReg |= (u16)(dataBit << count);
        count++;
        ps2BitCount = count;
        if (count == 11) { // When 11 bits collected, evaluate parity/stop and extract the data byte.
            stopBit = (u8)((ps2ShiftReg >> 10) & 0x1);
            if (((ps2ShiftReg & 0x1) == 0) && (stopBit != 0)) { // If start bit is 0 and stop bit is 1, frame looks valid.
                u8 dataByte = (u8)((ps2ShiftReg >> 1) & 0xFF);
                raw_push(dataByte);
            }
            ps2BitCount = 0;
            ps2ShiftReg = 0;
        }
    }
    EXTI->PR = (1 << 11);
}

void PS2_Update(void) // Convert raw bytes to key events and handle special prefix bytes.
{
    u8 dataByte;
    PS2KeyEvent evt;
    while (raw_pop(&dataByte)) {
        // Consume raw bytes and map prefix codes to state flags or push events.
        if (dataByte == 0xE0) { // Extended code prefix, next byte is extended.
            ps2ExtendedPending = 1;
            continue;
        }
        if (dataByte == 0xF0) { // Break code prefix, next byte is a release event.
            ps2BreakPending = 1;
            continue;
        }
        // If not a prefix, build a PS2KeyEvent and push to event FIFO.
        evt.code       = dataByte;
        evt.isBreak    = ps2BreakPending;
        evt.isExtended = ps2ExtendedPending;
        evt_push(&g_evtFifo, &evt);
        // Reset the prefix flags after creating the event.
        ps2BreakPending    = 0;
        ps2ExtendedPending = 0;
    }
}

int PS2_Poll(PS2KeyEvent *evt)
{
    // Pop a parsed event if available; returns non-zero if an event was provided.
    return evt_pop(&g_evtFifo, evt);
}
