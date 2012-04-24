#include <string.h>

#include "dcf77.h"

dcf77_state_t dcf77_state = {
    .update = {
        .bit = 0,
        .errors = 0,
        .parity = 0,
        .dcf77 = {
            .call = 0,
            .summer_time = 0,
            .cest = 0,
            .cet = 0,
            .leap_second = 0,
            .year = 0,
            .month = 0,
            .day = 0,
            .day_of_week = 0,
            .hour = 0,
            .minute = 0,
        },
    },
    .callback = 0,
};

#define dcf77    (dcf77_state.update.dcf77)
#define bit      (dcf77_state.update.bit)
#define parity   (dcf77_state.update.parity)
#define errors   (dcf77_state.update.errors)
#define callback (dcf77_state.callback)

#define BETWEEN(value, low, high) ((value) >= (low) && (value) <= (high))

/* weights of the bits for hour, minute, day of month/week, month, year */
static const uint8_t weights[] = { 1, 2, 4, 8, 10, 20, 40, 80 };

static inline void update(uint8_t value) {
    /* check parity */
    if (bit == 28 || bit == 35 || bit == 58) {
        /* invalid if parity = 1 */
        if (parity)
            errors = 1;
        /* reset parity for next block */
        parity = 0;

    /* minute, hour, day, day_of_week, month, year */
    } else if (BETWEEN(bit, 21, 57)) {
        uint8_t offset, *p;

        #define time_helper(start, stop, which) \
        if (bit >= (start) && bit <= (stop)) { \
            offset = (start); \
            p = &dcf77.which; \
        }

        time_helper(21, 27, minute)
        time_helper(29, 34, hour)
        time_helper(36, 41, day)
        time_helper(42, 44, day_of_week)
        time_helper(45, 49, month)
        time_helper(50, 57, year)

        *p += value * weights[bit - offset];

    /* call bit, dst announcement, cest, cet, leap second announcement */
    } else if (BETWEEN(bit, 15, 19)) {
        *((uint8_t*)&dcf77) |= value << (bit - 15);

    #if 0
    /* Not implemented: civil warning, weatherbroadcast */
    } else if (BETWEEN(bit, 1, 14)) {
    #endif

    /* start of minute, always 0 */
    } else if (bit == 0) {
        if (value)
            errors = 1;

    /* start of encoded time, always 1 */
    } else if (bit == 20) {
        if (!value)
            errors = 1;
        /* first parity block starts at bit 20 */
        parity = 0;

    }
}

/* state update from new bit, reset or error */
void dcf77_update(int8_t value) {
    if (value == DCF77_NEW_MINUTE) {
        /* run callback if there are no errors and we have seen a full
         * minute (59 or 60 bits) */
        if (!errors && (bit == 59 || bit == 60))
            callback(&dcf77);

        /* reset update state */
        memset(&dcf77_state.update, 0, sizeof(dcf77_state.update));

        return;
    }

    if (errors) {
        return;
    }

    if (value != 0 && value != 1) {
        errors = 1;
        return;
    }

    /* calculate parity (parity bit is included) */
    parity ^= value;

    update(value);

    bit += 1;
}

void dcf77_init(dcf77_callback_t f) {
    callback = f;
}
