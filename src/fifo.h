#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>

/* fifo - simple ring buffer */

typedef void (*fifo_consumer_t)(void);

typedef struct {
    /* size of buffer */
    uint8_t size;
    /* start of buffer content */
    uint8_t start;
    /* first free position of buffer
     * if end == start the buffer is empty
     * if end + 1 % size == start the buffer is full
     * i.e. one byte of the buffer is always unused
     */
    uint8_t end;
    /* consumer is called whenever something new is put into the buffer */
    fifo_consumer_t consumer;
    /* the buffer */
    char buf[];
} fifo_t;

/* initialize fifo_t structure */
void fifo_init(fifo_t*, uint8_t);
/* query number of bytes stored in the fifo */
uint8_t fifo_length(fifo_t*);
/* return first byte from fifo and remove it */
char fifo_shift(fifo_t*);
/* write string into fifo (null terminated) */
void fifo_print(fifo_t*, const char*);
/* printf into fifo */
void fifo_printf(fifo_t*, const char *format, ...);

#endif
