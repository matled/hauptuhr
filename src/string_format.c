#ifndef DEFINE_INLINE

#include <stdarg.h>

#include "string_format.h"

/* helper macro to append a character to buf */
#define put_char(c) \
do { \
    ++n; \
    if (n < size) { \
        *(buf++) = (c); \
    } \
} while(0)

/* assuming buf != NULL, size > 0, format != NULL */
unsigned int string_format_va(char *buf, unsigned int size,
                              const char *format, va_list ap) {
    /* number of used bytes */
    unsigned int n = 0;
    /* current character */
    char c;
    /* number of leading zeros for %d and %u */
    uint8_t leading_zeros;
    /* helper variable for number formatting */
    uint8_t skip;
    /* variable for integers */
    unsigned int number;
    /* variable for strings */
    const char *str;

    for (;;) {
        c = *format++;

        /* end of string */
        if (!c)
            break;

        /* normal character */
        if (c != '%') {
            put_char(c);
            continue;
        }

        c = *format++;

        /* read number of leading zeros */
        leading_zeros = 0;
        while (c >= '0' && c <= '9') {
            /* note: this will actually ignore everything but the lowest
             * digit */
            leading_zeros = c - '0';
            c = *format++;
        }

        /* end of string */
        if (!c) {
            break;

        /* %S for conditional strings
         * first argument is a truth value, second argument the string
         * the string is printed if the truth value is true */
        } else if (c == 'S') {
            /* print string, i.e. reuse code of %s */
            if (va_arg(ap, int)) {
                goto print_string;
            /* do not print string, i.e. shift from ap */
            } else {
                va_arg(ap, const char*);
            }

        /* %s for strings */
        } else if (c == 's') {
            print_string:
            str = va_arg(ap, const char*);
            /* just copy the argument */
            while (*str) {
                put_char(*str);
                ++str;
            }

        /* %d for signed int */
        } else if (c == 'd') {
            number = va_arg(ap, int);
            /* handle negative integers here */
            if ((int)number < 0) {
                put_char('-');
                number = -(int)number;
                if (leading_zeros)
                    --leading_zeros;
            }
            /* and reuse code for unsigned integers */
            goto print_unsigned_int;

        /* %u for unsigned int */
        } else if (c == 'u') {
            number = va_arg(ap, unsigned int);

            print_unsigned_int:

            if (leading_zeros > 5) {
                skip = 0;
            } else if (leading_zeros) {
                skip = 5 - leading_zeros;
            } else {
                skip = 4;
            }

            /* NOTE: up to 5 digits only */
            for (unsigned int i = 10000; i; i = i / 10) {
                /* if skip is 0 or this digit is not 0 */
                if (skip == 0 || ((number / i) % 10)) {
                    /* print this digit */
                    put_char('0' + (number / i) % 10);
                    skip = 0;
                } else {
                    /* just skipped one digit */
                    --skip;
                }
            }

        /* unknown format character */
        } else {
            put_char(c);
        }
    }

    *buf = '\0';

    return n;
}

#else /* DEFINE_INLINE */

static inline unsigned int string_format(char *buf, unsigned int size,
                                         const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    size = string_format_va(buf, size, format, ap);
    va_end(ap);
    return size;
}

#endif /* DEFINE_INLINE */
