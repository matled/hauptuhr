/* dcf77signal: interpret dcf77 signals from receiver
 *
 * This module interprets the signal from the receiver.  The function
 * dcf77signal() should be called with 0 if the signal power is reducend and 1
 * if the signal power is not reduced anymore.  The callback passed to
 * dcf77signal_init is called with the interpreted signal:
 * - 0 for a 0 bit
 * - 1 for a 1 bit
 * - DCF77SIGNAL_NEW_MINUTE if a new minute starts
 * - DCF77SIGNAL_ERROR if invalid signals were received
 *
 * Options:
 * - DCF77SIGNAL_THROTTLE_ERRORS:
 *   Call the callback at least this many ticks apart with
 *   DCF77SIGNAL_ERROR.
 */
#ifndef DCF77SIGNAL_H
#define DCF77SIGNAL_H

#include <stdint.h>

/* new minute starts */
#define DCF77SIGNAL_NEW_MINUTE (2)
/* error in signal */
#define DCF77SIGNAL_ERROR (-1)

/* argument to callback is 0, 1, DCF77SIGNAL_NEW_MINUTE, DCF77SIGNAL_ERROR */
typedef void (*dcf77signal_callback_t)(int8_t);

/* set callback and initialize module */
void dcf77signal_init(dcf77signal_callback_t);

#endif
