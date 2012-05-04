#include <stdbool.h>
#include <stdint.h>

#include "led.h"
#include "hardware.h"
#include "thread.h"
#include "hauptuhr.h"
#include "ticks.h"
#include "uart.h"
#include "clock.h"

#include "pt.h"

#define SIGNAL_TIMEOUT 2

static struct {
    bool dcf77_error;
    bool dcf77;
    ticks_t last_signal;
} state;

static PT_THREAD(blink(struct pt *pt, uint8_t led, uint16_t on, uint16_t off,
        ticks_t * ticks)) {
    PT_BEGIN(pt);
    TICKS_RESET(*ticks);
    for (;;) {
        hardware_led(led, 1);
        PT_WAIT_UNTIL(pt, TICKS_DIFF(*ticks) >= on);
        hardware_led(led, 0);
        PT_WAIT_UNTIL(pt, TICKS_DIFF(*ticks) >= on + off);
        TICKS_RESET(*ticks);
    }
    PT_END(pt);
}

/* green led: clock status */
THREAD(green) {
    static struct pt blink_thread;
    static uint16_t t_on, t_off;
    static ticks_t ticks;
    static uint8_t last_state;

    THREAD_BEGIN();
    for (;;) {
        last_state = clock_get_state();

        switch (last_state) {
        /* requires adjustment, not synced
         * => fast blinking */
        case CLOCK_INITIAL:
            t_off = TICKS(0.2);
            t_on =  TICKS(0.2);
            break;
        /* requires adjustment, synced
         * => slow blinking */
        case CLOCK_SYNCED:
            t_off = TICKS(0.9);
            t_on =  TICKS(0.9);
            break;
        /* not synced, will start as soon as sync complete
         * => short on */
        case CLOCK_PENDING:
            t_off = TICKS(0.9);
            t_on =  TICKS(0.2);
            break;
        /* not synced, running from adjustment
         * => long on */
        case CLOCK_HEADLESS:
            t_off = TICKS(0.2);
            t_on =  TICKS(0.9);
            break;
        /* running & synced
         * => permanently on */
        case CLOCK_RUNNING:
            t_off = TICKS(0);
            t_on =  TICKS(60);
            break;
        }

        PT_INIT(&blink_thread);
        for (;;) {
            blink(&blink_thread, 0, t_on, t_off, &ticks);
            THREAD_YIELD();
            /* break if state has changed */
            if (last_state != clock_get_state())
                break;
        }
    }

    THREAD_END();
}

/* red led: dcf77 status */
THREAD(red) {
    static struct pt blink_thread;
    static ticks_t ticks;

    static ticks_t timeout;
    THREAD_BEGIN();
    for (;;) {
        /* dcf77 error */
        if (state.dcf77_error) {
            /* blink (fast) */
            PT_INIT(&blink_thread);
            for (;;) {
                blink(&blink_thread, 1, TICKS(0.1), TICKS(0.1), &ticks);
                if (state.dcf77_error) {
                    TICKS_RESET(timeout);
                    state.dcf77_error = 0;
                }
                if (TICKS_DIFF(timeout) >= TICKS(0.1 * 8)) {
                    break;
                }
                THREAD_YIELD();
            }
        }

        /* everything fine, received dcf77 signal and no errors since then */
        if (state.dcf77) {
            /* disable red led */
            hardware_led(1, 0);
            THREAD_WAIT_UNTIL(!state.dcf77);
        /* errors occurred */
        } else if (TICKS_VALID(state.last_signal)) {
            /* blink (slow) */
            PT_INIT(&blink_thread);
            for (;;) {
                blink(&blink_thread, 1, TICKS(0.2), TICKS(0.9), &ticks);
                THREAD_YIELD();
                if (state.dcf77_error || state.dcf77 ||
                    !TICKS_VALID(state.last_signal)) {
                    break;
                }
            }
        /* no signal */
        } else {
            /* enable red led */
            hardware_led(1, 1);
            THREAD_WAIT_UNTIL(state.dcf77_error || state.dcf77 ||
                TICKS_VALID(state.last_signal));
        }
    }
    THREAD_END()
}

/* invalidate last_signal if no signal has been received within 2 seconds
 * (1.9s should be maximum between two signals) */
THREAD(signal_timeout) {
    THREAD_BEGIN();
    for (;;) {
        THREAD_WAIT_UNTIL(TICKS_VALID(state.last_signal) &&
            TICKS_DIFF(state.last_signal) >= TICKS(2));
        TICKS_INVALIDATE(state.last_signal);
        /* invalidate dcf77 status too */
        state.dcf77 = false;
    }
    THREAD_END();
}

/* received a full minute */
void led_dcf77(void) {
    state.dcf77 = true;
}

/* received a single signal */
void led_dcf77_signal(void) {
    TICKS_RESET(state.last_signal);
}

/* error while receiving */
void led_dcf77_error(void) {
    state.dcf77_error = true;
    state.dcf77 = false;
}

void led_init(void) {
    TICKS_INVALIDATE(state.last_signal);
    THREAD_INIT(red);
    thread_register(&red);
    THREAD_INIT(green);
    thread_register(&green);
    THREAD_INIT(signal_timeout);
    thread_register(&signal_timeout);
}
