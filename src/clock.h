#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

#ifndef CLOCK_24HOURS
#define CLOCK_MINUTES (12 * 60)
#else
#define CLOCK_MINUTES (24 * 60)
#endif

/* wait if the number of steps is <= CLOCK_WAIT_STEPS */
#ifndef CLOCK_WAIT_STEPS
/* by default take the time it takes to adjust CLOCK_MINUTES */
#define CLOCK_WAIT_STEPS (int)((ADVANCE_DURATION * CLOCK_MINUTES + 59) / 60)
#endif

typedef struct {
    /* current time */
    int16_t time;
    /* time displayed by the clock */
    int16_t clock;
    /* state: INITIAL, HEADLESS, PENDING, SYNCED, RUNNING */
    uint8_t state;
    /* stored state from eeprom */
    uint16_t stored;
} clock_state_t;

extern clock_state_t clock_state;

/* CLOCK_INITIAL: initial state
 * clock is not set, time is unknown. */
#define CLOCK_INITIAL (1 << 0)
/* CLOCK_HEADLESS: running headless
 * clock is running and assumed to be correct, but time is unknown. */
#define CLOCK_HEADLESS (1 << 1)
/* CLOCK_PENDING: restored state
 * clock state restored but time is unknown, not running. */
#define CLOCK_PENDING (1 << 2)
/* CLOCK_SYNCED: time known but not running */
#define CLOCK_SYNCED (1 << 3)
/* CLOCK_RUNNING: time known and clock is set */
#define CLOCK_RUNNING (1 << 4)

void clock_init(void);
/* Set time (use this for authorative time).  Time is hour * 60 +
 * minute. */
void clock_set(uint16_t);
/* Advance time by one minute.  Use this if you actually don't know the
 * time but the time should be increased by one minute. */
void clock_advance(int8_t);
/* Advance the clock by one minute to change the clock state to the
 * correct time. */
void clock_adjust(void);
/* Stop advancing until clock_adjust is called again. Preserved through
 * reboot. */
void clock_stop(void);

/* Get current time and clock. */
#define clock_get_time() (clock_state.time)
#define clock_get_clock() (clock_state.clock)
#define clock_get_state() (clock_state.state)

#define clock_is_synchronized() \
    (clock_get_state() & (CLOCK_SYNCED | CLOCK_RUNNING))
#define clock_is_stopped() \
    (clock_get_state() & (CLOCK_INITIAL | CLOCK_SYNCED))

#endif
