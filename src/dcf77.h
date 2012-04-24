#ifndef DCF77_H
#define DCF77_H

#include <stdint.h>

typedef struct {
    /* Note: update function depends on the first 5 bits */
    uint8_t call : 1;
    uint8_t summer_time : 1;
    uint8_t cest : 1;
    uint8_t cet : 1;
    uint8_t leap_second : 1;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t day_of_week;
    uint8_t hour;
    uint8_t minute;
} dcf77_t;

typedef void (*dcf77_callback_t)(dcf77_t*);

typedef struct {
    dcf77_callback_t callback;
    /* update state, reset every new minute */
    struct {
        /* bit number */
        uint8_t bit;
        /* errors occured */
        uint8_t errors;
        /* parity bit */
        uint8_t parity;
        /* dcf77 data */
        dcf77_t dcf77;
    } update;
} dcf77_state_t;

extern dcf77_state_t dcf77_state;

/* The specified function is called for each minute with correct signal.
 * Don't use the passed pointer outside the callback. */
void dcf77_init(dcf77_callback_t);

/* new minute starts */
#define DCF77_NEW_MINUTE (2)
/* error in signal */
#define DCF77_ERROR (-1)

/* update dcf77 state: 0, 1, DCF77_NEW_MINUTE or DCF77_ERROR */
void dcf77_update(int8_t);

#endif
