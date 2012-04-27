#ifndef ADVANCE_H
#define ADVANCE_H

#ifndef ADVANCE_DURATION
#define ADVANCE_DURATION 0.2
#endif

#include <stdint.h>

void advance_init(void);
void advance(void);
int8_t advance_busy(void);
uint8_t advance_polarity(void);
void advance_set_polarity(uint8_t);

#endif
