#include <avr/interrupt.h>

void init_timer1(void) {
    /* CTC mode, no prescaler */
    TCCR1B |= _BV(WGM12) | _BV(CS10);
    /* compare should match 256 times per second
     * NOTE: for 16MHz this is exactly 62500, other frequencies might
     * yield worse results due to rounding errrors */
    OCR1A = F_CPU / 256;
    /* enable compare 1A interrupt */
    TIMSK |= _BV(OCIE1A);
}

volatile uint8_t timer1_intr = 0;
ISR(SIG_OUTPUT_COMPARE1A) {
    /* set one bit in timer1_intr */
    timer1_intr = ~(~timer1_intr & (~timer1_intr - 1));
}
