#include "thread.h"
#include "ticks.h"
#include "hauptuhr.h"

static uint16_t uptime_minutes;

THREAD(uptime) {
    static ticks_t ticks;
    THREAD_BEGIN();
    for (;;) {
        TICKS_RESET(ticks);
        THREAD_WAIT_UNTIL(TICKS_DIFF(ticks) >= TICKS(60));
        if (uptime_minutes != 0xffff)
            uptime_minutes += 1;
    }
    THREAD_END();
}

uint16_t uptime_get(void) {
    return uptime_minutes;
}

void uptime_init(void) {
    THREAD_INIT(uptime);
    thread_register(&uptime);
}
