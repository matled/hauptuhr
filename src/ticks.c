#include "thread.h"
#include "hardware.h"
#include "hauptuhr.h"
#include "ticks.h"

ticks_t ticks_global = 0;

THREAD(ticks_update) {
    THREAD_BEGIN();

    for (;;) {
        THREAD_WAIT_UNTIL(hardware_timer);

        /* disable interrupts, clear one bit, reenable interrupts */
        /* TODO: if hardware_timer == 255 at least 8 interrupts
         * occurred, i.e. interrupts may be lost */
        hardware_interrupt_disable();
        hardware_timer &= hardware_timer - 1;
        hardware_interrupt_enable();

        /* main tick counter */
        ++ticks_global;
    }

    THREAD_END();
}

void ticks_init(void) {
    THREAD_INIT(ticks_update);
    thread_register(&threads_busy, &ticks_update);
}
