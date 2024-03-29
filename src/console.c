#include <stdbool.h>

#include "clock.h"
#include "controller.h"
#include "eeprom.h"
#include "uart.h"
#include "hardware.h"
#include "uptime.h"

static void print_status(void) {
    uart_print(
        "status\r\n"
        " state ");
    switch (clock_get_state()) {
    case CLOCK_INITIAL:
        uart_print("initial");
        break;
    case CLOCK_PENDING:
        uart_print("pending");
        break;
    case CLOCK_SYNCED:
        uart_print("synced");
        break;
    case CLOCK_HEADLESS:
        uart_print("headless");
        break;
    case CLOCK_RUNNING:
        uart_print("running");
        break;
    }
    uart_printf("\r\n"
        " time  %2u:%2u\r\n"
        " clock %2u:%2u\r\n"
        " up    %um\r\n",
        clock_get_time() / 60,
        clock_get_time() % 60,
        clock_get_clock() / 60,
        clock_get_clock() % 60,
        uptime_get());
}

static void print_adjust(void) {
    uart_puts("clock adjust");
    controller_adjust();
}

static void print_auto_adjust(void) {
    uart_puts("clock auto adjust");
    controller_auto_adjust();
}

static void print_stop(void) {
    uart_puts("clock stop");
    controller_stop();
}

static void print_dcf77(bool value) {
    uart_printf("DCF77 enabled=%u\r\n", value);
    if (value) {
        hardware_dcf77_enable();
    } else {
        hardware_dcf77_disable();
    }
}

static void print_dcf77_verbose(bool value) {
    uart_printf("DCF77 verbose=%u\r\n", value);
    controller_verbose(value);
}

static void print_version(void) {
    uart_puts("hauptuhr " VERSION);
}

static void print_eeprom(void) {
    uart_printf("eeprom v=%u l=%u\r\n", eeprom_load(), eeprom_state.location);
}

static void print_help(void) {
    uart_print(
        "Commands:\r\n"
        " s status\r\n"
        " a adjust\r\n"
        " A auto adjust\r\n"
        " z stop\r\n"
        " d/D dcf77\r\n"
        " o/O verbose\r\n"
        " e eeprom\r\n"
        " r reboot\r\n"
        " v version\r\n"
        " h help\r\n"
    );
}

static void input(char c) {
    switch (c) {
    case 's': print_status(); break;
    case 'a': print_adjust(); break;
    case 'A': print_auto_adjust(); break;
    case 'z': print_stop(); break;
    case 'd': print_dcf77(1); break;
    case 'D': print_dcf77(0); break;
    case 'o': print_dcf77_verbose(1); break;
    case 'O': print_dcf77_verbose(0); break;
    case 'v': print_version(); break;
    case 'e': print_eeprom(); break;
    case 'r': hardware_reboot(); break;
    case 'h': print_help(); break;
    default:  print_help(); break;
    }
}

void console_init(void) {
    uart_init(input);
    print_version();
}
