#include <avr/io.h>

#include "thread.h"
#include "uart.h"
#include "fifo.h"
#include "test.h"

uart_state_t uart_state;

THREAD(uart_send) {
    THREAD_BEGIN();
    for (;;) {
        THREAD_WAIT_UNTIL(fifo_length(&uart_state.fifo) &&
            bit_is_set(UCSR0A, UDRE0));
        UDR0 = fifo_shift(&uart_state.fifo);
    }
    THREAD_END();
}

THREAD(uart_recv) {
    THREAD_BEGIN();
    for (;;) {
        THREAD_WAIT_UNTIL(bit_is_set(UCSR0A, RXC0));
        uart_state.receive(UDR0);
    }
    THREAD_END();
}

void uart_init(uart_receive_callback_t callback) {
    /* initialize receive fifo */
    fifo_init(&uart_state.fifo, sizeof(uart_state.buf));
    /* store callback */
    uart_state.receive = callback;
    /* set baud rate */
    #if !defined(BAUD) && defined(UART_BAUDRATE)
    #define BAUD UART_BAUDRATE
    #elif !defined(BAUD)
    #define BAUD 19200
    #endif
    #include <util/setbaud.h>
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    /* set mode 8N1 */
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
    /* enable send and receive */
    UCSR0B = _BV(TXEN0) | _BV(RXEN0);
    /* enable pull-up for rx pin */
    PORTD |= _BV(PORTD0);

    /* register threads */
    thread_register(&threads_busy, &uart_send);
    thread_register(&threads_busy, &uart_recv);
}
