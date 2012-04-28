#include <stdlib.h>

#include "hardware.h"
#include "advance.h"
#include "blink.h"
#include "dcf77.h"
#include "dcf77signal.h"
#include "thread.h"
#include "ticks.h"
#include "uart.h"
#include "eeprom.h"
#include "clock.h"
#include "controller.h"

thread_t *threads_busy = NULL;
thread_t *threads_tick = NULL;

static uint16_t value;

static void console(char c) {
    switch (c) {
    case 'a':
        controller_adjust();
        break;
    case 'z':
        controller_stop();
        break;
    case 'e':
        value = eeprom_load();
        uart_printf("eeprom v=%u l=%u\r\n", value, eeprom_state.location);
        break;
    case 'r':
        hardware_led(1, -1);
        uart_print(".\r\n");
        break;
    }
}

int main(void) {
    ticks_init();
    uart_init(console);
    uart_print("hauptuhr " VERSION "\r\n");
    blink_init();
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
