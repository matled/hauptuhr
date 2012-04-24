#include <avr/io.h>

#include "advance.h"
#include "uart.h"
#include "hauptuhr.h"
#include "thread.h"
#include "timer.h"

struct {
    uint8_t polarity;
    uint8_t working;
} advance_state = {
    .polarity = 0,
    .working = 0,
};

THREAD(advance_thread) {
    static uint16_t timer;
    THREAD_BEGIN();

    for (;;) {
        THREAD_WAIT_UNTIL(advance_state.working);
        if (advance_state.polarity) {
            PORTB |= _BV(PORTB1);
        } else {
            PORTB |= _BV(PORTB2);
        }
        TIMER_RESET();
        THREAD_WAIT_UNTIL(TIMER_DIFF() >= TIME(0.2));
        PORTB &= ~(_BV(PORTB1) | _BV(PORTB2));
        advance_state.polarity ^= 1;
        advance_state.working = 0;
    }

    THREAD_END();
}

void advance(void) {
    advance_state.working = 1;
}

int8_t advance_busy(void) {
    return advance_state.working;
}

void advance_init(void) {
    /* disabled by default */
    PORTB &= ~(_BV(PORTB1) | _BV(PORTB2));
    /* output pins */
    DDRB |= _BV(DDB1) | _BV(DDB2);

    thread_register(&threads_busy, &advance_thread);
}
