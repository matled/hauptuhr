#include <stdlib.h>

#include "hardware.h"
#include "advance.h"
#include "dcf77.h"
#include "dcf77signal.h"
#include "thread.h"
#include "ticks.h"
#include "uart.h"
#include "eeprom.h"
#include "clock.h"
#include "controller.h"
#include "button.h"
#include "led.h"
#include "console.h"

thread_t *threads_busy = NULL;
thread_t *threads_tick = NULL;

int main(void) {
    ticks_init();
    console_init();
    button_init();
    led_init();
    controller_init();

    hardware_interrupt_enable();

    for (;;) {
        /* CALL: busy threads */
        THREAD_RUN_ALL(threads_busy);
        /* CALL: ticking threads */
        THREAD_RUN_ALL(threads_tick);
    }
    return 0;
}
