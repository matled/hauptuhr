#include "hardware.h"

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
