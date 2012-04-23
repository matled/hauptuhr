#include <avr/io.h>
#include <stdint.h>
#include "thread.h"
#include "test.h"
#include "timer.h"

static uint16_t blink1_arg = TPS / 2;
static uint16_t blink2_arg = TPS;

THREAD(blink1) {
    static uint16_t time;
    THREAD_BEGIN();
    for (;;) {
        time = ticks;
        THREAD_YIELD_UNTIL((ticks - time) >= *THREAD_ARG(uint16_t*));
        PORTC ^= _BV(PORTC4);
    }
    THREAD_END();
}

THREAD(blink2) {
    static uint16_t time;
    THREAD_BEGIN();
    time = ticks;
    for (;;) {
        THREAD_WAIT_UNTIL((ticks - time) >= *THREAD_ARG(uint16_t*));
        PORTC ^= _BV(PORTC5);
        time = ticks;
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
    thread_register(&threads_tick, &blink2);
}
