#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

#include "timer.h"

/* interval of 0.2s = 1s/5 with 256 ticks per second */
#ifndef CLOCK_INTERVAL
#define CLOCK_INTERVAL 256 / 5
#endif
/* wait if the number of steps is <= CLOCK_WAIT_STEPS */
#ifndef CLOCK_WAIT_STEPS
#define CLOCK_WAIT_STEPS 10
#endif

#ifndef CLOCK_24HOURS
#define CLOCK_MINUTES (12 * 60)
#else
#define CLOCK_MINUTES (24 * 60)
#endif

typedef void (*clock_advance_callback_t)(uint8_t);
typedef void (*clock_save_callback_t)(uint16_t);

/* CLOCK_INITIAL: initial state
 * clock is not set, time is unknown. */
#define CLOCK_INITIAL 0
/* CLOCK_HEADLESS: running headless
 * clock is running and assumed to be correct, but time is unknown. */
#define CLOCK_HEADLESS 1
/* CLOCK_PENDING: restored state
 * clock state restored but time is unknown, not running. */
#define CLOCK_PENDING 2
/* CLOCK_SYNCED: time known but not running */
#define CLOCK_SYNCED 3
/* CLOCK_RUNNING: time known and clock is set */
#define CLOCK_RUNNING 4

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
static inline int16_t clock_get_time(void);
static inline int16_t clock_get_clock(void);
static inline uint8_t clock_get_state(void);

#define clock_is_synchronized() (clock_get_state() == CLOCK_SYNCED || clock_get_state() == CLOCK_RUNNING)
#define clock_is_stopped() (clock_get_state() == CLOCK_INITIAL || clock_get_state() == CLOCK_SYNCED)

#define DEFINE_INLINE
#include "clock.c"
#undef DEFINE_INLINE

#endif
