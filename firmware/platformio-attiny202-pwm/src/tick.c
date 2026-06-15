#include "tick.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#define TICK_CLOCK_HZ 3333333UL
#define TICK_RATE_HZ  1000UL

static volatile uint16_t tick_pending = 0;

void tick_init_1ms(void) {
    /*
     * TCB0 periodic interrupt every 1 ms.
     *
     * CCMP = clock / rate - 1
     *
     * 3,333,333 / 1000 ≈ 3333 counts per ms
     */
    TCB0.CTRLA = 0;

    TCB0.CTRLB = TCB_CNTMODE_INT_gc;

    TCB0.CCMP = (uint16_t)((TICK_CLOCK_HZ / TICK_RATE_HZ) - 1);

    TCB0.CNT = 0;

    TCB0.INTCTRL = TCB_CAPT_bm;

    TCB0.CTRLA =
        TCB_CLKSEL_CLKDIV1_gc |
        TCB_ENABLE_bm;
}

bool tick_take_1ms(void) {
    bool had_tick = false;

    uint8_t sreg = SREG;
    cli();

    if (tick_pending > 0) {
        tick_pending--;
        had_tick = true;
    }

    SREG = sreg;

    return had_tick;
}

ISR(TCB0_INT_vect) {
    TCB0.INTFLAGS = TCB_CAPT_bm;

    if (tick_pending < 0xFFFF) {
        tick_pending++;
    }
}