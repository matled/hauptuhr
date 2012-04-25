#include <avr/interrupt.h>

#include "thread.h"
#include "hardware.h"
#include "hauptuhr.h"
#include "timer.h"

static volatile uint8_t timer_tick = 0;
uint16_t timer_ticks = 0;

THREAD(timer_update) {
    THREAD_BEGIN();

    for (;;) {
        THREAD_WAIT_UNTIL(timer_tick);

        /* disable interrupts, clear one bit, reenable interrupts */
        /* TODO: if timer_tick == 255 at least 8 interrupts
         * occurred, i.e. interrupts may be lost */
        cli();
        timer_tick &= timer_tick - 1;
        sei();

        /* main tick counter */
        ++timer_ticks;
    }

    THREAD_END();
}

void timer_init(void) {
    hardware_timer_init();
    THREAD_INIT(timer_update);
    thread_register(&threads_busy, &timer_update);
}

/* TODO: move this to hardware.c */
ISR(SIG_OUTPUT_COMPARE1A) {
    timer_tick = ~(~timer_tick & (~timer_tick - 1));
}
