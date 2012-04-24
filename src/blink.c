#include <avr/io.h>
#include <stdint.h>
#include "thread.h"
#include "hauptuhr.h"
#include "timer.h"

static uint16_t blink1_arg = TIME(1);
static uint16_t blink2_arg = TIME(0.5);

THREAD(blink1) {
    static uint16_t timer;
    THREAD_BEGIN();
    for (;;) {
        TIMER_RESET();
        THREAD_YIELD_UNTIL(TIMER_DIFF() >= *THREAD_ARG(uint16_t*));
        PORTC ^= _BV(PORTC4);
    }
    THREAD_END();
}

THREAD(blink2) {
    static uint16_t timer;
    THREAD_BEGIN();
    TIMER_RESET();
    for (;;) {
        THREAD_WAIT_UNTIL(TIMER_DIFF() >= *THREAD_ARG(uint16_t*));
        PORTC ^= _BV(PORTC5);
        TIMER_RESET();
        THREAD_YIELD();
    }
    THREAD_END();
}

void blink_init(void) {
    /* led ports as output */
    DDRC |= _BV(DDC4) | _BV(DDC5);

    blink1.arg = &blink1_arg;
    THREAD_INIT(blink1);
    blink2.arg = &blink2_arg;
    THREAD_INIT(blink2);

    thread_register(&threads_tick, &blink1);
    //thread_register(&threads_tick, &blink2);
}
