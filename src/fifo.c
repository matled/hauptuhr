#include <stdarg.h>

#ifdef USE_PROGMEM
#include <avr/pgmspace.h>
#define str_dereference(p) pgm_read_byte(p)
#else
#define str_dereference(p) (*(p))
#endif

#include "string_format.h"

#include "fifo.h"

/* a modulo b returning always a positive value */
#define MOD(a, b) ((((a) % (b)) + (b)) % (b))
/* the buffer is full if end + 1 is the same as start */
#define FULL(fifo) ((fifo->end + 1) % fifo->size == fifo->start)

void fifo_init(fifo_t *fifo, uint8_t size) {
    fifo->size = size;
    fifo->start = 0;
    fifo->end = 0;
    fifo->consumer = 0;
}

uint8_t fifo_length(fifo_t *fifo) {
    return MOD(fifo->end - fifo->start, fifo->size);
}

char fifo_shift(fifo_t *fifo) {
    if (fifo->start == fifo->end)
        return '\0';
    char byte = fifo->buf[fifo->start];
    fifo->start = (fifo->start + 1) % fifo->size;
    return byte;
}

void fifo_print(fifo_t *fifo, const char *bytes) {
    const char *p = bytes;
    /* copy until \0 or the buffer is full */
    while (str_dereference(p) && !FULL(fifo)) {
        fifo->buf[fifo->end] = str_dereference(p);
        fifo->end = (fifo->end + 1) % fifo->size;
        ++p;
    }

    /* if there was something copied into the fifo call consumer */
    if (p != bytes && fifo->consumer)
        fifo->consumer();
}

void fifo_printf(fifo_t *fifo, const char *format, ...) {
    /* cannot do anything if the fifo is full already */
    if (FULL(fifo)) {
        return;
    }

    /* for printf we have to ensure that the free area of the buffer
     * does not wrap around the end. */

    /* buffer empty, reset start and end to 0 */
    if (fifo->start == fifo->end) {
        fifo->start = fifo->end = 0;
    /* the free area wraps around iff
     * start < 0 (at least the first position is free) and
     * if end > start (there is another free position after start
     * because end points to the first free position) */
    } else if (fifo->start != 0 && fifo->start < fifo->end) {
        /* move the content of the buffer to the beginning.  the first
         * position is unused and therefore nothing is overwritten
         * before it is moved. */
        for (uint8_t i = 0; i < fifo->end - fifo->start; ++i) {
            fifo->buf[i] = fifo->buf[fifo->start + i];
        }
        fifo->end = fifo->end - fifo->start;
        fifo->start = 0;
    }

    va_list ap;
    va_start(ap, format);
    int n = string_format_va(fifo->buf + fifo->end,
        fifo->size - fifo_length(fifo), format, ap);
    va_end(ap);

    if (n < 0)
        return;

    if (n > fifo->size - fifo_length(fifo) - 1) {
        fifo->end += fifo->size - fifo_length(fifo) - 1;
    } else {
        /* note: end cannot wrap around here */
        fifo->end += n;
    }

    if (n > 0 && fifo->consumer)
        fifo->consumer();
}
