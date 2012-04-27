#include <stdlib.h>

#include "hardware.h"
#include "advance.h"
#include "blink.h"
#include "dcf77.h"
#include "dcf77signal.h"
#include "thread.h"
#include "ticks.h"
#include "uart.h"
#include "eeprom.h"
#include "clock.h"

thread_t *threads_busy = NULL;
thread_t *threads_tick = NULL;

static uint16_t value;

static void console(char c) {
    switch (c) {
    case '=':
        clock_set(clock_get_time());
        break;
    case ' ':
        clock_advance(1);
        break;
    case 'a':
        clock_adjust();
        break;
    case 'z':
        clock_stop();
        break;
    case 'A':
        uart_printf("a:polarity=%d working=%d\r\n",
            advance_polarity(), advance_busy());
    case 'e':
        value = eeprom_load();
        uart_printf("eeprom v=%u l=%u\r\n", value, eeprom_state.location);
        break;
    case 'E':
        eeprom_store(value + 1);
        break;
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
        uart_puts("DCF77 1");
        break;
    case 0:
        uart_puts("DCF77 0");
        break;
    case DCF77SIGNAL_NEW_MINUTE:
        uart_puts("DCF77 N");
        break;
    default:
        uart_puts("DCF77 error");
        break;
    }
}

void minute_done(dcf77_t *dcf77) {
    uart_printf("DCF77: 20%2u-%2u-%2u %2u:%2u\r\n",
        dcf77->year, dcf77->month, dcf77->day, dcf77->hour, dcf77->minute);
    clock_set(dcf77->hour * 60 + dcf77->minute);
}

int main(void) {
    ticks_init();
    uart_init(console);

    uart_print("hauptuhr " VERSION "\r\n");

    advance_init();
    eeprom_init();
    blink_init();
    dcf77signal_init(dcf77_print);
    dcf77_init(minute_done);

    /* requires advance, uart, eeprom */
    clock_init();
    //clock_set(clock_get_time());

    hardware_interrupt_enable();

    for (;;) {
        /* CALL: busy threads */
        THREAD_RUN_ALL(threads_busy);
        /* CALL: ticking threads */
        THREAD_RUN_ALL(threads_tick);
    }
    return 0;
}
