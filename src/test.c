#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/version.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "thread.h"
#include "uart.h"
#include "timer.h"
#include "blink.h"

uint16_t ticks = -1;
thread_t *threads_busy = NULL;
thread_t *threads_tick = NULL;

void foo(char c) {
    switch (c) {
    case 'a':
        PORTC ^= _BV(PORTC5);
        uart_print(".\r\n");
        break;
    }
}

int main(void) {
    uart_init(foo);
    timer_init();
    blink_init();

    wdt_disable();

    sei();

    uart_print("hauptuhr " VERSION "\r\n");
    for (;;) {
        /* CALL: busy threads */
        THREAD_RUN_ALL(threads_busy);

        if (timer_tick) {
            wdt_reset();

            /* disable interrupts, clear one bit, reenable interrupts */
            /* TODO: if timer_tick == 255 at least 8 interrupts
             * occurred, i.e. interrupts may be lost */
            cli();
            timer_tick &= timer_tick - 1;
            sei();

            /* main tick counter */
            ++ticks;

            /* CALL: ticking threads */
            THREAD_RUN_ALL(threads_tick);
        }
    }
    return 0;
}
