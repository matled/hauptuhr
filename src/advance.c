#include "hardware.h"
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
            hardware_advance1();
        } else {
            hardware_advance2();
        }
        TIMER_RESET();
        THREAD_WAIT_UNTIL(TIMER_DIFF() >= TIME(0.2));
        hardware_advance_disable();
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

uint8_t advance_polarity(void) {
    return advance_state.polarity;
}

void advance_init(void) {
    hardware_advance_init();
    thread_register(&threads_busy, &advance_thread);
}
