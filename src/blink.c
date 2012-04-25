#include <stdint.h>
#include "hardware.h"
#include "thread.h"
#include "hauptuhr.h"
#include "timer.h"

THREAD(blink1) {
    static uint16_t timer;
    THREAD_BEGIN();
    for (;;) {
        TIMER_RESET();
        THREAD_YIELD_UNTIL(TIMER_DIFF() >= TIME(1));
        hardware_led(0, -1);
    }
    THREAD_END();
}

void blink_init(void) {
    /* led ports as output */
    hardware_led_init();
    THREAD_INIT(blink1);
    thread_register(&threads_tick, &blink1);
}
