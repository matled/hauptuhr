#ifndef BLINK_H
#define BLINK_H

#include "thread.h"

extern thread_t blink1;
extern thread_t blink2;

void blink_init(void);

#endif
