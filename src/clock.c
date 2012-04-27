#include "advance.h"
#include "clock.h"
#include "eeprom.h"
#include "hauptuhr.h"
#include "thread.h"
#include "uart.h"

/* mod CLOCK_MINUTES for values > -CLOCK_MINUTES */
#define MOD_TIME(time) (((time) + CLOCK_MINUTES) % CLOCK_MINUTES)
/* mod CLOCK_MINUTES for values >= 0 */
#define MOD_TIME_P(time) ((time) % CLOCK_MINUTES)

clock_state_t clock_state = {
    .time = 0,
    .clock = 0,
    .state = CLOCK_INITIAL,
};

static void clock_print_state(const char *str) {
    uart_printf("C:%s t=%u c=%u s=%u p=%u\r\n",
        str,
        clock_state.time,
        clock_state.clock,
        clock_state.state,
        advance_polarity());
}

static void save(void) {
    uint16_t s;
    if (clock_state.state == CLOCK_RUNNING || clock_state.state == CLOCK_PENDING) {
        s = clock_state.clock;
    } else {
        s = 0x07ff;
    }
    s |= (advance_polarity() << 11);
    if (clock_state.stored != s) {
        clock_state.stored = s;
        eeprom_store(clock_state.stored);
    }
}

THREAD(clock_thread) {
    THREAD_BEGIN();

    for (;;) {
        /* advance clock, if */
        THREAD_WAIT_UNTIL(
            /* advance clock is in state RUNNING or HEADLESS */
            (clock_state.state == CLOCK_RUNNING || clock_state.state == CLOCK_HEADLESS) &&
            /* advance() is ready */
            !advance_busy() &&
            /* waiting for the clock to become correct would take too long */
            MOD_TIME(clock_state.clock - clock_state.time) > (CLOCK_WAIT_STEPS)
        );

        clock_print_state("thread1");

        /* advance clock */
        advance();
        THREAD_WAIT_WHILE(advance_busy());
        /* update clock state */
        clock_state.clock = MOD_TIME_P(clock_state.clock + 1);
        /* save new state (including new polarity) */
        save();
        clock_print_state("thread2");
    }

    THREAD_END();
}

void clock_init(void) {
    THREAD_INIT(clock_thread);
    thread_register(&threads_busy, &clock_thread);

    clock_state.stored = eeprom_load();

    /* no state saved */
    if (clock_state.stored & 0xc000) {
        return;
    }

    /* valid clock state stored */
    if ((clock_state.stored & 0x07ff) <= CLOCK_MINUTES) {
        clock_state.time = clock_state.clock = clock_state.stored & 0x07ff;
        clock_state.state = CLOCK_PENDING;

        advance_set_polarity((~(clock_state.stored >> 11)) & 1);
        advance();
    } else {
        advance_set_polarity((clock_state.stored >> 11) & 1);
    }

    clock_print_state("init");
}

void clock_set(uint16_t time) {
    uart_printf("C:set(%u)\r\n", time);
    clock_state.time = MOD_TIME(time);

    switch (clock_state.state) {
    case CLOCK_INITIAL:
        clock_state.state = CLOCK_SYNCED;
        break;
    case CLOCK_HEADLESS:
        clock_state.state = CLOCK_RUNNING;
        /* Assume clock was correct by setting it to one minute in the
         * past, i.e. advance once now. */
        clock_state.clock = MOD_TIME(clock_state.time - 1);
        break;
    case CLOCK_PENDING:
        clock_state.state = CLOCK_RUNNING;
        break;
    }

    save();
    clock_print_state("set");
}

void clock_adjust(void) {
    uart_printf("C:adjust\r\n");
    switch (clock_state.state) {
    case CLOCK_INITIAL:
    case CLOCK_HEADLESS:
    case CLOCK_PENDING:
        clock_state.state = CLOCK_HEADLESS;
        /* Set time to one minute in the future. */
        clock_state.time = MOD_TIME_P(clock_state.clock + 1);
        break;
    case CLOCK_SYNCED:
    case CLOCK_RUNNING:
        clock_state.state = CLOCK_RUNNING;
        /* Set clock to one minute in the past.  If we are currently
         * advancing this will do nothing. */
        clock_state.clock = MOD_TIME(clock_state.time - 1);
        break;
    }

    save();
    clock_print_state("adjust");
}

void clock_advance(int8_t value) {
    uart_printf("C:advance\r\n");
    clock_state.time = MOD_TIME(clock_state.time + value);
    clock_print_state("advance");
}

void clock_stop(void) {
    uart_printf("C:stop\r\n");
    switch (clock_state.state) {
    case CLOCK_HEADLESS:
    case CLOCK_PENDING:
        clock_state.state = CLOCK_INITIAL;
        break;
    case CLOCK_RUNNING:
        clock_state.state = CLOCK_SYNCED;
        break;
    }

    save();
    clock_print_state("stop");
}
