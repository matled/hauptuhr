/* Custom printf implementation with limited format characters:
 * %s: string
 * %S: conditional string
 *     first argument is interpreted as truth value, the second argument
 *     is a string.  if the first argument is true the second is
 *     printed.
 * %d: signed integer
 * %u: unsigned integer
 * padding in form of leading zeros is available only for integers.
 * limitations: only 5 digits of numbers are printed (enough for 16 bit
 * integers).
 *
 * A trailing \0 is printed to the buffer in every case.
 * The return value is the number of bytes written without the \0.
 */
#ifndef STRING_FORMAT_H
#define STRING_FORMAT_H

#include <stdarg.h>
#include <stdint.h>

/* arguments: buffer, size, format string, argument list */
unsigned int string_format_va(char*, unsigned int, const char*, va_list);
static inline unsigned int string_format(char*, unsigned int, const char*,
                                         ...);

#define DEFINE_INLINE
#include "string_format.c"
#undef DEFINE_INLINE

#endif
