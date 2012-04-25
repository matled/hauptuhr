#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdbool.h>
#include <avr/io.h>

#include "timer.h"

static inline void hardware_timer_init(void) {
    /* use 16 bit timer/counter 1 */
    /* CTC mode */
    TCCR1B |= _BV(WGM12);
    /* 256 prescaling */
    TCCR1B |= _BV(CS12);
    /* compare value */
    OCR1A = F_CPU / TPS / 256;
    /* compare match interrupt */
    TIMSK1 |= _BV(OCIE1A);
}

static inline void hardware_uart_init(void) {
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
}

static inline bool hardware_uart_recv_ready(void) {
    return bit_is_set(UCSR0A, RXC0);
}

/* call this only after hardware_uart_recv_ready returns 1 */
static inline char hardware_uart_recv(void) {
    return UDR0;
}

static inline bool hardware_uart_send_ready(void) {
    return bit_is_set(UCSR0A, UDRE0);
}

/* call this only after hardware_uart_recv_ready returns 1 */
static inline void hardware_uart_send(char c) {
    UDR0 = c;
}

static inline void hardware_advance_init(void) {
    /* disabled by default */
    PORTB &= ~(_BV(PORTB1) | _BV(PORTB2));
    /* output pins */
    DDRB |= _BV(DDB1) | _BV(DDB2);
}

static inline void hardware_advance_disable(void) {
    PORTB &= ~(_BV(PORTB1) | _BV(PORTB2));
}

static inline void hardware_advance1(void) {
    PORTB |= _BV(PORTB1);
}

static inline void hardware_advance2(void) {
    PORTB |= _BV(PORTB2);
}

static inline void hardware_dcf77_init(void) {
    /* dcf77 enable/disable is a output pin */
    DDRB |= _BV(DDB0);
    /* enable dcf77 */
    PORTB &= ~_BV(PORTB0);
}

static inline bool hardware_dcf77(void) {
    return bit_is_set(PIND, PORTD3);
}

static inline void hardware_led_init(void) {
    DDRC |= _BV(DDC4) | _BV(DDC5);
}

static inline void hardware_led(uint8_t led, int8_t value) {
    if (led == 0) {
        led = _BV(PORTC4);
    } else {
        led = _BV(PORTC5);
    }

    if (value == -1) {
        PORTC ^= led;
    } else if (value == 0) {
        PORTC &= ~led;
    } else {
        PORTC |= led;
    }
}


/* TODO: interrupts may be used in one of two ways:
 * either it just wakes the cpu up from sleeping
 * or it signals threads to be run
 */

#if 0
static inline void hardware_uart_recv_enable(void) {
    /* UCSRB |= _BV(RXCIE); */
}

static inline void hardware_uart_recv_disable(void) {
    /* UCSRB &= ~_BV(RXCIE); */
}

static inline void hardware_uart_send_enable(void) {
    /* UCSRB |= _BV(UDRIE); */
}

static inline void hardware_uart_send_disable(void) {
    /* UCSRB &= ~_BV(UDRIE); */
}
#endif

#endif
