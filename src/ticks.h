#ifndef TICKS_H
#define TICKS_H

#include <stdint.h>

/* ticks per second */
#define TICKS_PER_SECOND 100

typedef uint16_t ticks_t;
/* every tick one bit in this variable is set */
extern ticks_t ticks_global;
/* init module */
void ticks_init(void);

#define TICKS(seconds) ((uint16_t)((seconds) * (TICKS_PER_SECOND)))

#define TICKS_INVALID(ticks) ((ticks_global - (ticks)) >= 0x8000)
#define TICKS_VALID(ticks) (!TICKS_INVALID(ticks))
#define TICKS_INVALIDATE(ticks) do { \
    (ticks) = ticks_global - 0x8000; \
} while (0)

#define TICKS_NO_WRAP(ticks) do { \
    if (TICKS_INVALID(ticks)) \
        TICKS_INVALIDATE(ticks); \
} while (0);

#define TICKS_DIFF(ticks) (ticks_global - (ticks))
#define TICKS_RESET(ticks) do { \
    (ticks) = ticks_global; \
} while (0)

#endif
