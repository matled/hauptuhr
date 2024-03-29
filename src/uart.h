/* module for uart
 * provides a fifo for writing and a callback whenever a character is
 * read
 */
#ifndef UART_H
#define UART_H

#ifndef UART_FIFO_SIZE
#define UART_FIFO_SIZE (128)
#endif

#ifdef USE_PROGMEM
#include <avr/pgmspace.h>
#define _UART_STR PSTR
#else
#define _UART_STR
#endif

#include "fifo.h"

typedef void (*uart_receive_callback_t)(char);

typedef struct {
    uart_receive_callback_t receive;
    fifo_t fifo;
    uint8_t buf[UART_FIFO_SIZE];
} uart_state_t;

extern uart_state_t uart_state;

/* initialize this module and set callback */
void uart_init(uart_receive_callback_t);

/* set callback */
#define uart_set_callback(callback) (uart_state.receive = (callback))

/* print string */
#define uart_print(str) fifo_print(&uart_state.fifo, _UART_STR(str))
/* print string (only string literals) plus "\r\n" */
#define uart_puts(str) uart_print(str "\r\n")
/* printf */
#define uart_printf(format, ...) \
    fifo_printf(&uart_state.fifo, _UART_STR(format), __VA_ARGS__)

#endif
