#include "hardware.h"
#include "advance.h"
#include "uart.h"
#include "hauptuhr.h"
#include "thread.h"
#include "ticks.h"

struct {
    uint8_t polarity;
    uint8_t working;
} advance_state = {
    .polarity = 0,
    .working = 0,
};

THREAD(advance_thread) {
    static ticks_t ticks;
    THREAD_BEGIN();

    for (;;) {
        THREAD_WAIT_UNTIL(advance_state.working);
        if (advance_state.polarity) {
            hardware_advance1();
        } else {
            hardware_advance2();
        }
        TICKS_RESET(ticks);
        THREAD_WAIT_UNTIL(TICKS_DIFF(ticks) >= TICKS(ADVANCE_DURATION));
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

void advance_set_polarity(uint8_t value) {
    advance_state.polarity = value;
}

void advance_init(void) {
    THREAD_INIT(advance_thread);
    thread_register(&advance_thread);
}
