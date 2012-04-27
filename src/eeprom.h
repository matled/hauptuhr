#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

#define EEPROM_INVALID 0xffff

typedef struct {
    /* current value or value to be stored */
    uint16_t value;
    /* current location or the location written to */
    uint16_t location;
    /* first byte & 0x80 has to be equal to this value to be valid */
    uint8_t valid;
    /* save in progress */
    uint8_t start_working;
} eeprom_state_t;

extern eeprom_state_t eeprom_state;

/* load value from eeprom */
uint16_t eeprom_load(void);
/* store value in eeprom (14 bits only) */
void eeprom_store(uint16_t);
/* callback function for writing to eeprom */
//void eeprom_write(void);
void eeprom_init(void);

#endif
