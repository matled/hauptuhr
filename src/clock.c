#include "advance.h"
#include "clock.h"
#include "eeprom.h"
#include "hauptuhr.h"
#include "thread.h"

/* mod CLOCK_MINUTES for values > -CLOCK_MINUTES */
#define MOD_TIME(time) (((time) + CLOCK_MINUTES) % CLOCK_MINUTES)
/* mod CLOCK_MINUTES for values >= 0 */
#define MOD_TIME_P(time) ((time) % CLOCK_MINUTES)

clock_state_t clock_state = {
    .time = 0,
    .clock = 0,
    .state = CLOCK_INITIAL,
};

static void save(void) {
    uint16_t s;
    s = advance_polarity() << 11;
    if (clock_state.state & (CLOCK_RUNNING | CLOCK_PENDING)) {
        s |= clock_state.clock;
    } else {
        s |= 0x07ff;
    }
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
            /* advance clock in state RUNNING or HEADLESS */
            (clock_state.state & (CLOCK_RUNNING | CLOCK_HEADLESS)) &&
            /* advance() is ready */
            !advance_busy() &&
            /* waiting for the clock to become correct would take too long */
            MOD_TIME(clock_state.clock - clock_state.time) > (CLOCK_WAIT_STEPS)
        );

        /* advance clock */
        advance();
        THREAD_WAIT_WHILE(advance_busy());
        /* update clock state */
        clock_state.clock = MOD_TIME_P(clock_state.clock + 1);
        /* save new state (including new polarity) */
        save();
    }

    THREAD_END();
}

void clock_init(void) {
    THREAD_INIT(clock_thread);
    thread_register(&clock_thread);

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
}

void clock_set(uint16_t time) {
    clock_state.time = MOD_TIME(time);

    if (clock_state.state & (CLOCK_INITIAL | CLOCK_SYNCED)) {
        clock_state.state = CLOCK_SYNCED;
        return;
    }

    if (clock_state.state == CLOCK_HEADLESS) {
        /* Assume clock was correct by setting it to one minute in the
         * past, i.e. advance once now. */
        clock_state.clock = MOD_TIME(clock_state.time - 1);
    }
    clock_state.state = CLOCK_RUNNING;

    save();
}

void clock_adjust(void) {
    if (clock_state.state & (CLOCK_RUNNING | CLOCK_SYNCED)) {
        clock_state.state = CLOCK_RUNNING;
    /* initial, pending, headless */
    } else {
        clock_state.state = CLOCK_HEADLESS;
    }

    /* Set clock to one minute in the past.  If we are currently
     * advancing this will do nothing. */
    clock_state.clock = MOD_TIME(clock_state.time - 1);

    save();
}

void clock_advance(int8_t value) {
    clock_state.time = MOD_TIME(clock_state.time + value);
}

void clock_stop(void) {
    if (clock_state.state & (CLOCK_RUNNING | CLOCK_SYNCED)) {
        clock_state.state = CLOCK_SYNCED;
    /* initial, pending, headless */
    } else {
        clock_state.state = CLOCK_INITIAL;
    }

    save();
}
