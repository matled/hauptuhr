#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdbool.h>

void controller_adjust(void);
void controller_auto_adjust(void);
void controller_stop(void);
void controller_verbose(bool);
void controller_init(void);

#endif
