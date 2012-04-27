/*
 * eeprom - store 14 bits in eeprom with wear leveling
 *
 * This module allows to store 14 bits in the eeprom.  The value is not
 * stored at a fixed position but will rotate the position for every
 * call to store a new value.
 *
 * Two subsequent positions are used to store the value.  The lower 7
 * bits of each byte are used to store the value.
 *
 * The highest bits of the bytes have to differ to form a valid value.
 * The first two bytes with this property indicate if the first bit has
 * to be 1 or the second.  After that all following positions are
 * scanned for another valid position and the last one is used.
 *
 * For storing the next invalid position is used.  If this wraps around
 * the bits indicating valid/invalid are toggled.
 *
 * As long as one byte can be written atomically this should yield
 * either the value before storing the new one or the newly stored
 * value.
 */

#include "eeprom.h"
#include "thread.h"
#include "hardware.h"
#include "hauptuhr.h"
#include "uart.h"

eeprom_state_t eeprom_state;

#define state eeprom_state

/* load value from eeprom.  do not call this function if store is in
 * progress.  it is recommended to call this function only once. */
uint16_t eeprom_load(void) {
    /* valid:
     * 0xff => no valid position found yet
     * 0x00 => valid positions: a = 0b0xxx_xxx && b = 0b1xxx_xxxx
     * 0x80 => valid positions: a = 0b1xxx_xxx && b = 0b0xxx_xxxx
     */
    state.valid = 0xff;
    state.location = 0;
    state.value = EEPROM_INVALID;

    for (uint16_t i = 0; i < hardware_eeprom_size() - 1; i += 2) {
        uint8_t a = hardware_eeprom_read(i);
        uint8_t b = hardware_eeprom_read(i + 1);

        /* if this position is invalid (i.e. highest bit is the same) */
        if ((a & 0x80) == (b & 0x80)) {
            /* no other valid position */
            if (state.valid == 0xff) {
                /* eeprom has not been used yet, i.e. is invalid */
                return state.value;
            } else {
                /* use the last valid position. */
                break;
            }
        }

        /* first valid position decides which following positions are
         * valid. */
        if (state.valid == 0xff) {
            state.valid = a & 0x80;
        /* this position is invalid with regard to the first valid position */
        } else if ((a & 0x80) != state.valid) {
            /* use the last valid position */
            break;
        }

        /* update location and value */
        state.location = i;
        state.value = (a & 0x7f) << 7 | (b & 0x7f);
    }

    return state.value;
}

THREAD(eeprom_write) {
    THREAD_BEGIN();

    for (;;) {
        THREAD_WAIT_UNTIL(state.start_working);

        /* update location and valid */

        /* if state is valid write to the next position */
        if (state.valid != 0xff) {
            state.location = (state.location + 2) % hardware_eeprom_size();
        }
        /* if state is invalid or wrapped around toggle valid */
        if (state.location == 0) {
            /* mask with 0x80 in case valid was 0xff */
            state.valid = (state.valid & 0x80) ^ 0x80;
        }

        state.start_working = 0;

        /* loop until no start_working call was made during THREAD_WAIT_UNTIL */
        for (;;) {
            /* wait for eeprom to be ready for write */
            THREAD_WAIT_UNTIL(hardware_eeprom_ready());
            /* restart if eeprom_store has been called in between */
            if (state.start_working)
                continue;

            /* write first byte */
            hardware_eeprom_write(state.location,
                ((state.value >> 7) & 0x7f) | state.valid);

            /* wait for eeprom to be ready for write */
            THREAD_WAIT_UNTIL(hardware_eeprom_ready());
            /* restart if eeprom_store has been called in between */
            if (state.start_working)
                continue;

            /* write second byte */
            hardware_eeprom_write(state.location + 1,
                (state.value & 0x7f) | (state.valid ^ 0x80));

            /* successfully stored the value */
            break;
        }

        /* disable eeprom interrupt */
        hardware_eeprom_write_disable();
    }

    THREAD_END();
}

void eeprom_init(void) {
    THREAD_INIT(eeprom_write);
    thread_register(&threads_busy, &eeprom_write);

    while (!hardware_eeprom_ready())
        ;
    eeprom_load();
}

/* store up to 14 bits in eeprom.  extra bits are ignored.
 * eeprom_load has to be called before. */
void eeprom_store(uint16_t value) {
    /* trim value to 14 bits */
    state.value = value & 0x3fff;
    /* store state for callback */
    state.start_working = 1;
    /* enable writing */
    hardware_eeprom_write_enable();
}
