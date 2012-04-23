#include <avr/interrupt.h>

#include "timer.h"

volatile uint8_t timer_tick = 0;

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
}

ISR(SIG_OUTPUT_COMPARE0A) {
    timer_tick = ~(~timer_tick & (~timer_tick - 1));
}

ISR(SIG_OUTPUT_COMPARE1A) {
    timer_tick = ~(~timer_tick & (~timer_tick - 1));
}
