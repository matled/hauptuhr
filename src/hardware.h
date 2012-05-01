#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>

#include "ticks.h"

/* initialize hardware */
void hardware_init(void);

/* enable interrupts */
static inline void hardware_interrupt_enable(void) {
    sei();
}
/* disable interrupts */
static inline void hardware_interrupt_disable(void) {
    cli();
}
/* for every tick one bit is set in this variable */
extern uint8_t hardware_timer;
/* true if a byte can be read from the uart */
bool hardware_uart_recv_ready(void);
/* read byte from uart
 * call this only after hardware_uart_recv_ready returns 1 */
char hardware_uart_recv(void);
/* true if a byte can be send to the uart */
bool hardware_uart_send_ready(void);
/* write on byte to uart
 * call this only after hardware_uart_recv_ready returns 1 */
void hardware_uart_send(char c);
/* advance clock */
void hardware_advance_disable(void);
void hardware_advance1(void);
void hardware_advance2(void);
/* enable/disable dcf77 */
void hardware_dcf77_enable(void);
void hardware_dcf77_disable(void);
/* read dcf77 signal */
bool hardware_dcf77(void);
/* get size of eeprom */
static inline uint16_t hardware_eeprom_size(void) {
    return AVR_EEPROM_SIZE;
}
/* true if last write has been completed and read/write may be used */
bool hardware_eeprom_ready(void);
/* read one byte from eeprom */
uint8_t hardware_eeprom_read(uint16_t address);
/* write one byte to eeprom */
void hardware_eeprom_write(uint16_t address, uint16_t value);
/* read status of buttons*/
bool hardware_button1(void);
bool hardware_button2(void);
void hardware_led(uint8_t led, int8_t value);

/* TODO: interrupts may be used to put the cpu into sleep when nothing
 * happens instead of polling all the time */
#define hardware_eeprom_write_enable() do { } while (0)
#define hardware_eeprom_write_disable() do { } while (0)
/* UCSRB |= _BV(RXCIE); */
#define hardware_uart_recv_enable() do { } while (0)
#define hardware_uart_recv_disable() do { } while (0)
/* UCSRB |= _BV(UDRIE); */
#define hardware_uart_send_enable() do { } while (0)
#define hardware_uart_send_disable() do { } while (0)

#endif
