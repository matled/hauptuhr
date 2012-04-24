#ifndef TIMER_H
#define TIMER_H

/* ticks per second */
#define TPS 100
/* every tick one bit in this variable is set */
extern uint16_t timer_ticks;
/* init module */
void timer_init(void);

#define TIME(value) ((uint16_t)((value) * TPS))

#define TIMER_NO_WRAP() do { \
    if (timer_ticks - timer > 0x8000) \
        timer = timer_ticks - 0x8000; \
} while (0);

#define TIMER_DIFF() (timer_ticks - timer)
#define TIMER_RESET() do { timer = timer_ticks; } while (0)

#endif
