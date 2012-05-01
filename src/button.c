#include "thread.h"
#include "ticks.h"
#include "controller.h"
#include "hardware.h"
#include "hauptuhr.h"
#include "button.h"

/* button1: adjust
 * While button1 is pressed the clock is adjusted every BUTTON_ADJUST_TIME
 * seconds.  If the button is pressed long enough to adjust
 * BUTTON_AUTO_ADJUST_COUNT times, auto_adjust is started.
 */
THREAD(button1) {
    static ticks_t ticks;
    static uint8_t count;
    
    THREAD_BEGIN();
    for (;;) {
        /* store count of adjusts for auto_adjust */
        count = 0;
        /* if button1 is pressed */
        while (hardware_button1()) {
            /* adjust clock */
            controller_adjust();
            count += 1;
            /* start timer */
            TICKS_RESET(ticks);
            /* wait for BUTTON_ADJUST_TIME or until button1 is released */
            THREAD_YIELD_UNTIL(TICKS_DIFF(ticks) >= TICKS(BUTTON_ADJUST_TIME) ||
                !hardware_button1());
            if (!hardware_button1()) {
                break;
            }
            /* if auto_adjust threshold is reached */
            if (count >= BUTTON_AUTO_ADJUST_COUNT) {
                /* enable auto_adjust */
                controller_auto_adjust();
                /* and wait until button1 is released */
                THREAD_YIELD_UNTIL(!hardware_button1());
                break;
            }
        }
        THREAD_YIELD();
    }
    THREAD_END();
}

/* button2: simply stop the clock */
THREAD(button2) {
    THREAD_BEGIN();
    for (;;) {
        if (hardware_button2()) {
            controller_stop();
        }
        THREAD_YIELD();
    }
    THREAD_END();
}

void button_init(void) {
    THREAD_INIT(button1);
    thread_register(&threads_busy, &button1);
    THREAD_INIT(button2);
    thread_register(&threads_busy, &button2);
}
