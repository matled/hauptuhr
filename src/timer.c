#include <avr/interrupt.h>

#include "thread.h"
#include "hauptuhr.h"
#include "timer.h"

static volatile uint8_t timer_tick = 0;
uint16_t timer_ticks = 0;

THREAD(timer_update) {
    THREAD_BEGIN();

    for (;;) {
        THREAD_WAIT_UNTIL(timer_tick);

        /* disable interrupts, clear one bit, reenable interrupts */
        /* TODO: if timer_tick == 255 at least 8 interrupts
         * occurred, i.e. interrupts may be lost */
        cli();
        timer_tick &= timer_tick - 1;
        sei();

        /* main tick counter */
        ++timer_ticks;
    }

    THREAD_END();
}

void timer_init(void) {
    #if 1
    /* 16 bit timer/counter 1 */
    /* CTC mode */
    TCCR1B |= _BV(WGM12);
    /* 256 prescaling */
    TCCR1B |= _BV(CS12);
    /* compare value */
    OCR1A = F_CPU / TPS / 256;
    /* compare match interrupt */
    TIMSK1 |= _BV(OCIE1A);
    #else
    /* 8 bit timer/counter 0 */
    /* CTC mode */
    TCCR0A |= _BV(WGM01);
    /* 1024 prescaling */
    TCCR0B |= _BV(CS02) | _BV(CS00);
    /* compare value */
    OCR0A = F_CPU / TPS / 1024;
    /* compare match interrupt */
    TIMSK0 |= _BV(OCIE0A);
    #endif

    thread_register(&threads_busy, &timer_update);
}

ISR(SIG_OUTPUT_COMPARE0A) {
    timer_tick = ~(~timer_tick & (~timer_tick - 1));
}

ISR(SIG_OUTPUT_COMPARE1A) {
    timer_tick = ~(~timer_tick & (~timer_tick - 1));
}
