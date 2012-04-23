#ifndef TIMER_H
#define TIMER_H

/* ticks per second */
#define TPS 100
/* every tick one bit in this variable is set */
extern volatile uint8_t timer_tick;
/* init module */
void timer_init(void);

#endif
