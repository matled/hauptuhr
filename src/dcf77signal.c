#include "dcf77signal.h"
#include "hardware.h"
#include "thread.h"
#include "hauptuhr.h"
#include "uart.h"
#include "timer.h"

#define VAR TIME(0.05)
/* helper macro: check if x and y differ at most by VAR */
#define ALMOST(x, y) \
    ((x) <= (y) + (VAR) && (y) <= (x) + (VAR))

static struct {
    dcf77signal_callback_t callback;
} dcf77signal_state;

#define state dcf77signal_state

THREAD(dcf77signal) {
    static uint16_t timer;

    THREAD_BEGIN();

    for (;;) {
        while (!hardware_dcf77()) {
            TIMER_NO_WRAP();
            THREAD_YIELD();
        }

        /* check if last power reduction was 2s ago
         * in this case a new minute starts
         */
        if (ALMOST(TIMER_DIFF(), TIME(2))) {
            state.callback(DCF77SIGNAL_NEW_MINUTE);
        /* check if last power reduction was not 1s ago
         * 1s would mean a bit was received and nothing has to be done
         * otherwise an error occurred
         */
        } else if (!ALMOST(TIMER_DIFF(), TIME(1))) {
            state.callback(DCF77SIGNAL_ERROR);
        }

        TIMER_RESET();

        while (hardware_dcf77()) {
            TIMER_NO_WRAP();
            THREAD_YIELD();
        }

        /* check if power reduction was 100ms
         * in this case a 0 bit was received
         */
        if (ALMOST(TIMER_DIFF(), TIME(0.1))) {
            state.callback(0);
        /* check if power reduction was 200ms
         * in this case a 1 bit was received
         */
        } else if (ALMOST(TIMER_DIFF(), TIME(0.2))) {
            state.callback(1);
        } else {
            state.callback(DCF77SIGNAL_ERROR);
        }
    }

    THREAD_END();
}

void dcf77signal_init(dcf77signal_callback_t callback) {
    hardware_dcf77_init();
    /* store callback */
    dcf77signal_state.callback = callback;
    /* register thread */
    THREAD_INIT(dcf77signal);
    thread_register(&threads_tick, &dcf77signal);
}   
