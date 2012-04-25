#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdlib.h>

#include "hardware.h"
#include "advance.h"
#include "blink.h"
#include "dcf77.h"
#include "dcf77signal.h"
#include "thread.h"
#include "timer.h"
#include "uart.h"

thread_t *threads_busy = NULL;
thread_t *threads_tick = NULL;

static void console(char c) {
    switch (c) {
    case 'a':
        advance();
        break;
    case 'A':
        uart_printf("a:polarity=%d working=%d\r\n",
            advance_polarity(), advance_busy());
    case 'r':
        hardware_led(1, -1);
        uart_print(".\r\n");
        break;
    }
}

static void dcf77_print(int8_t signal) {
    dcf77_update(signal);

    switch (signal) {
    case 1:
        uart_print("1");
        break;
    case 0:
        uart_print("0");
        break;
    case DCF77SIGNAL_NEW_MINUTE:
        uart_print("N");
        break;
    default:
        uart_print("E");
        break;
    }
}

void minute_done(dcf77_t *dcf77) {
    uart_printf("DCF77: 20%2u-%2u-%2u %2u:%2u\r\n",
        dcf77->year, dcf77->month, dcf77->day, dcf77->hour, dcf77->minute);
}

int main(void) {
    advance_init();
    advance();

    blink_init();
    dcf77signal_init(dcf77_update);
    dcf77_init(minute_done);
    timer_init();
    uart_init(console);

    wdt_disable();

    sei();

    uart_print("hauptuhr " VERSION "\r\n");
    for (;;) {
        /* CALL: busy threads */
        THREAD_RUN_ALL(threads_busy);
        /* CALL: ticking threads */
        THREAD_RUN_ALL(threads_tick);
    }
    return 0;
}
