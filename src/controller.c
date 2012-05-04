#include <stdint.h>
#include <stdbool.h>

#include "advance.h"
#include "clock.h"
#include "dcf77.h"
#include "dcf77signal.h"
#include "eeprom.h"
#include "hauptuhr.h"
#include "thread.h"
#include "ticks.h"
#include "uart.h"
#include "led.h"

#include "controller.h"

static struct {
    ticks_t minute;
    ticks_t last_dcf77;
    ticks_t last_dcf77_error;
    uint16_t time;
    bool verbose;
    bool auto_adjust;
    bool summer_time;
    bool cet;
} state;

THREAD(minute) {
    THREAD_BEGIN();
    for (;;) {
        while (TICKS_INVALID(state.minute) ||
            TICKS_DIFF(state.minute) < TICKS(60)) {
            TICKS_NO_WRAP(state.minute);
            THREAD_YIELD();
        }

        if (state.summer_time && clock_get_time() % 60 == 59) {
            state.summer_time = 0;
            /* CET to CEST: advance one hour */
            if (state.cet) {
                clock_advance(61);
                /* CEST to CET: go back one hour */
            } else {
                clock_advance(-59);
            }
        } else {
            clock_advance(1);
        }

        TICKS_RESET(state.minute);
    }
    THREAD_END();
}

THREAD(auto_adjust) {
    THREAD_BEGIN();
    for (;;) {
        if (state.auto_adjust) {
            clock_adjust();
        }
        THREAD_YIELD();
    }
    THREAD_END();
}

static void dcf77_signal(int8_t signal) {
    dcf77_update(signal);

    if (signal == DCF77SIGNAL_ERROR) {
        led_dcf77_error();
    } else {
        led_dcf77_signal();
    }

    if (signal == DCF77SIGNAL_ERROR) {
        /* throttle the number of messages shown to one per second
         * NOTE: This function is called only if a signal is received
         * and therefore last_dcf77_error may wrap around.  This could
         * lead to a missed message.  As this message is quite spammy
         * and informational this is ok. */
        if (TICKS_DIFF(state.last_dcf77_error) >= TICKS(1)) {
            uart_puts("DCF77 error");
            TICKS_RESET(state.last_dcf77_error);
        }
        return;
    }

    if (!state.verbose) {
        return;
    }

    /* quite internal stuff of dcf77 but nice verbose output */
    dcf77_t *d = &dcf77_state.update.dcf77;
    uart_printf("DCF77 %u %S%2u "
        "%u%u%u%u%u "
        "20%2u-%2u-%2uT%2u:%2u %u\r\n",
        signal,
        dcf77_state.update.errors, "E ",
        (uint8_t)(dcf77_state.update.bit - 1) & 0x3f,
        d->call, d->summer_time, d->cest, d->cet, d->leap_second,
        d->year, d->month, d->day, d->hour, d->minute,
        d->day_of_week);
}

static void dcf77_time(dcf77_t *dcf77) {
    uart_printf("DCF77: 20%2u-%2u-%2u %2u:%2u",
        dcf77->year, dcf77->month, dcf77->day, dcf77->hour, dcf77->minute);

    uint16_t now = dcf77->hour * 60 + dcf77->minute;
    /* if now == time + 1 two successive minutes have been received */
    if (now == (state.time + 1) % (24 * 60)) {
        /* print number how many ticks the timer was off for one minute */
        uart_printf(" (%d)",
            TICKS_DIFF(state.last_dcf77) - (TICKS_PER_SECOND * 60));

        /* update summer_time and cet */
        state.summer_time = dcf77->summer_time;
        state.cet = dcf77->cet;

        /* set clock */
        clock_set(dcf77->hour * 60 + dcf77->minute);
        /* reset ticks for new minute */
        TICKS_RESET(state.minute);

        /* set the led module know that a dcf77 signal was received
         * successfully */
        led_dcf77();
    } else {
        uart_print(" (ign)");
    }
    uart_print("\r\n");

    /* reset ticks and store time for next minute */
    TICKS_RESET(state.last_dcf77);
    state.time = now;
}

void controller_adjust(void) {
    clock_adjust();
    state.auto_adjust = 0;
    TICKS_RESET(state.minute);
}

void controller_auto_adjust(void) {
    state.auto_adjust = 1;
}

void controller_stop(void) {
    clock_stop();
    state.auto_adjust = 0;
    TICKS_INVALIDATE(state.minute);
}

void controller_verbose(bool value) {
    state.verbose = value;
}

void controller_init(void) {
    dcf77signal_init(dcf77_signal);
    dcf77_init(dcf77_time);

    advance_init();
    eeprom_init();
    clock_init();

    THREAD_INIT(minute);
    thread_register(&minute);
    TICKS_INVALIDATE(state.minute);

    THREAD_INIT(auto_adjust);
    thread_register(&auto_adjust);
}
