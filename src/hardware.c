#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ticks.h"
#include "hardware.h"

#if !defined(__AVR_ATmega8__) && \
    !defined(__AVR_ATmega88__) && \
    !defined(__AVR_ATmega168__)
#error Unsupported CPU
#endif

#ifdef __AVR_ATmega8__
#define TIMSK1 TIMSK
#define UBRR0H UBRRH
#define UBRR0L UBRRL
#define UCSR0A UCSRA
#define UCSR0B UCSRB
#define UDR0 UDR
#define UDRE0 UDRE
#define RXC0 RXC
#define RXEN0 RXEN
#define TXEN0 TXEN
#define EEMPE EEMWE
#define EEPE EEWE
#endif

/* timer */
static inline void hardware_timer_init(void) {
    /* use 16 bit timer/counter 1 */
    /* CTC mode */
    TCCR1B |= _BV(WGM12);
    /* no prescaling */
    TCCR1B |= _BV(CS10);
    /* compare value */
    OCR1A = (F_CPU) / (TICKS_PER_SECOND);
    /* compare match interrupt */
    TIMSK1 |= _BV(OCIE1A);
}

uint8_t hardware_timer;
ISR(SIG_OUTPUT_COMPARE1A) {
    hardware_timer = ~(~hardware_timer & (~hardware_timer - 1));
}

/* uart */
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
    #ifdef __AVR_ATmega8__
    UCSRC = _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1);
    #else
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
    #endif
    /* enable send and receive */
    UCSR0B = _BV(TXEN0) | _BV(RXEN0);
    /* enable pull-up for rx pin */
    PORTD |= _BV(PORTD0);
}

bool hardware_uart_recv_ready(void) {
    return bit_is_set(UCSR0A, RXC0);
}

char hardware_uart_recv(void) {
    return UDR0;
}

bool hardware_uart_send_ready(void) {
    return bit_is_set(UCSR0A, UDRE0);
}

void hardware_uart_send(char c) {
    UDR0 = c;
}

/* advance */
static inline void hardware_advance_init(void) {
    /* disabled by default */
    PORTB &= ~(_BV(PORTB1) | _BV(PORTB2));
    /* output pins */
    DDRB |= _BV(DDB1) | _BV(DDB2);
}

void hardware_advance_disable(void) {
    PORTB &= ~(_BV(PORTB1) | _BV(PORTB2));
}

void hardware_advance1(void) {
    PORTB |= _BV(PORTB1);
}

void hardware_advance2(void) {
    PORTB |= _BV(PORTB2);
}

/* dcf77 */
static inline void hardware_dcf77_init(void) {
    /* dcf77 enable/disable is a output pin */
    DDRB |= _BV(DDB0);
    /* enable dcf77 */
    PORTB &= ~_BV(PORTB0);
}

void hardware_dcf77_enable(void) {
    PORTB &= ~_BV(PORTB0);
}

void hardware_dcf77_disable(void) {
    PORTB |= _BV(PORTB0);
}

bool hardware_dcf77(void) {
    return bit_is_set(PIND, PORTD3);
}

/* eeprom */
uint8_t hardware_eeprom_read(uint16_t address) {
    while (!hardware_eeprom_ready())
        ;
    EEAR = address;
    EECR |= _BV(EERE);
    return EEDR;
}

void hardware_eeprom_write(uint16_t address, uint16_t value) {
    while (!hardware_eeprom_ready())
        ;
    EEAR = address;
    EEDR = value;
    EECR |= _BV(EEMPE);
    EECR |= _BV(EEPE);
}

bool hardware_eeprom_ready(void) {
    return bit_is_clear(EECR, EEPE);
}

/* button */
static inline void hardware_button_init(void) {
    /* enable pull-up */
    PORTD |= _BV(PORTD6) | _BV(PORTD7);
}

bool hardware_button1(void) {
    return bit_is_clear(PIND, PORTD7);
}

bool hardware_button2(void) {
    return bit_is_clear(PIND, PORTD6);
}

/* led */
static inline void hardware_led_init(void) {
    /* output pins */
    DDRC |= _BV(DDC4) | _BV(DDC5);
}

void hardware_led(uint8_t led, int8_t value) {
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

/* init */
void hardware_init(void) {
    hardware_led_init();
    hardware_dcf77_init();
    hardware_advance_init();
    hardware_timer_init();
    hardware_uart_init();
    hardware_button_init();
}
