#include <stdlib.h>

#include "button.h"
#include "console.h"
#include "controller.h"
#include "hardware.h"
#include "led.h"
#include "thread.h"
#include "ticks.h"
#include "uptime.h"

thread_t *threads = NULL;

void thread_register(thread_t *thread) {
    thread->next = threads;
    threads = thread;
}

int main(void) {
    /* initialize modules */
    hardware_init();
    ticks_init();
    uptime_init();
    console_init();
    button_init();
    led_init();
    controller_init();

    /* enable interrupts */
    hardware_interrupt_enable();

    for (;;) {
        /* run all threads */
        THREAD_RUN_ALL(threads);
    }

    return 0;
}
