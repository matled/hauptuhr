#include <stdint.h>
#include "hardware.h"
#include "thread.h"
#include "hauptuhr.h"
#include "ticks.h"

THREAD(blink1) {
    static ticks_t ticks;
    THREAD_BEGIN();
    for (;;) {
        TICKS_RESET(ticks);
        THREAD_YIELD_UNTIL(TICKS_DIFF(ticks) >= TICKS(1));
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
