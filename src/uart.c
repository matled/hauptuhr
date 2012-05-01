#include "hardware.h"
#include "thread.h"
#include "uart.h"
#include "fifo.h"
#include "hauptuhr.h"

uart_state_t uart_state;

THREAD(uart_send) {
    THREAD_BEGIN();
    for (;;) {
        THREAD_WAIT_UNTIL(fifo_length(&uart_state.fifo) &&
            hardware_uart_send_ready());
        hardware_uart_send(fifo_shift(&uart_state.fifo));
    }
    THREAD_END();
}

THREAD(uart_recv) {
    THREAD_BEGIN();
    for (;;) {
        THREAD_WAIT_UNTIL(hardware_uart_recv_ready());
        uart_state.receive(hardware_uart_recv());
    }
    THREAD_END();
}

void uart_init(uart_receive_callback_t callback) {
    /* initialize receive fifo */
    fifo_init(&uart_state.fifo, sizeof(uart_state.buf));
    /* store callback */
    uart_state.receive = callback;
    /* register threads */
    THREAD_INIT(uart_send);
    thread_register(&threads_busy, &uart_send);
    THREAD_INIT(uart_recv);
    thread_register(&threads_busy, &uart_recv);
}
