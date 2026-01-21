/********************************************************************************************************************/

#include <sys/assert.h>
#include <sys/cdefs.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

/********************************************************************************************************************/
/*
 * BUGS:
 *   - Currently, passing in a character for an integer format option will
 * be treated as a 32bit wide integer.  This can cause some unusual results
 * with things such as:
 *
 * int i = 0xDEADBEEF;
 * char c = 'A';
 * int j = 0x0F00C0DE;
 *
 * printf("%02X", c);
 *
 * To work around this cast the char to an int:
 *
 * printf("%02X", (int)c);
 * 
 * 64-Bit integer support is busted. :/
 */

 /********************************************************************************************************************/
/*
 * Convert a base value to ascii.
 *
 * Parameters:
 * val		- Value to convert.
 * base		- Number base (e.g. 10 for decimal, or 16 for hexadecimal).
 * upper 	- Upper case flag for digits beyond 10 in non decimal counting.
 * buf  	- The buffer to place the output.
 * buflen	- Length of the buffer.
 *
 * Remarks:
 * For bases that exceed 10, letters (starting with the letter 'a') are
 * used.  If the case parameter is non-zero then upper case letters are
 * used, if 0 (zero) then lower case leters are used.
 *
 * IMPORTANT:
 * For effency this function starts at the end of the string and works its
 * way backwards to the begining of the string.
 *
 * Return Value:
 * Pointer to the start of the number in the string.
 */
static char *base_to_ascii(uint64_t val, int base, bool upper, char *buf, size_t buflen)
{
    size_t i = buflen - 1;
    char c, cont = 1;
    int d;

    /* We write our string backwards. */
    buf[i] = '\0';
    --i;

    for (;;)
    {
        d = val % base;
        val /= base;

        if (d >= 10)
            c = (d - 10) + (upper ? 'A' : 'a');
        else
            c = d + '0';

        cont = (i > 0);
        buf[i--] = c;

        if (!cont || (val == 0))
            break;
    };

    return !cont ? buf : (buf + i + 1);
}

/********************************************************************************************************************/

#define FLAGS_UPPER    0x0001
#define FLAGS_NOSIGN   0x0002
#define FLAGS_MAXINT   0x0004
#define FLAGS_LONGINT  0x0008
#define FLAGS_LONGLONG 0x0010
#define FLAGS_SHORT    0x0020

/********************************************************************************************************************/
/*
 * Our own "lite" vsnprintf() function.
 *
 * Only supports a limited amount of the normal printf() style formating
 * options, these are:
 * c   - Single character
 * d/i - Signed integer value
 * u   - Unsigned integer value
 * x/X - Unsigned hexidecimal value
 * o   - Unsigned octal number
 * p   - Pointer (upper case hexidecimal with a '0x' prefix)
 * s   - NULL terminated C string.
 * 
 * j   - 64-bit flag for integers.  (E.g. %ju)
 * ll  - 64-bit flag for integers.  (E.g. %lld)
 * 
 *    64-BIT INTS ARE BROKEN!
 *
 * Formatting options include:
 * - Padding with optional leading zeros for numbers.
 * - Sign extending for signed number values.
 *
 * Loosly based on the BSD vsnprintf() function.
 */
int vsnprintf(char *sbuf, size_t slen, const char *fmt, va_list args)
{
    const char *fp = fmt;
    size_t base, width;
    char nbuf[64], *p;
    size_t s = 0, n;
    char pad, sign;
    int64_t val;
    int flags;
    
    for (;;)
    {
        while ((*fp != '%') && (*fp != '\0'))
        {
            if (s < slen)
                sbuf[s++] = *fp++;
            else
                goto exit;
        }

        if (*fp == '\0')
            break;

        sign = '\0';
        flags = 0;
        width = 0;
        pad = ' ';

reswitch:
        ++fp; /* Skip */

        switch (*fp)
        {
        default:
        case '\0':
        case '%':
            sbuf[s++] = *fp;

            if (*fp == '\0')
                goto exit;

            continue;

        case '0':
            if (width > 0)
                width *= 10;
            else
                pad = '0';

            goto reswitch;

        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            width = width * 10 + (*fp - '0');
            goto reswitch;

        case '+':
            sign = '+';
            goto reswitch;

        case 'j':
            flags |= FLAGS_MAXINT;
            goto reswitch;

        case 'c':
            sbuf[s++] = (char)va_arg(args, int);
            break;

        case 'h':
            goto reswitch;

        case 'l':
            if (flags & FLAGS_LONGINT)
            {
                flags &= ~FLAGS_LONGINT;
                flags |= FLAGS_LONGLONG;
            }
            else
                flags |= FLAGS_LONGINT;
            goto reswitch;

        case 'o':
            flags |= FLAGS_NOSIGN;
            base = 8;
            goto number;

        case 'u':
            flags |= FLAGS_NOSIGN;
            /* Fall through */

        case 'd':
        case 'i':
            base = 10;
            goto number;

        case 'x':
            flags &= ~FLAGS_UPPER;
            goto hex;

        case 'p': /* formated pointer */
            sbuf[s] = '\0';
            p = _s_strncat(sbuf + s, (slen - s), "0x", 3);

            ASSERT(p != NULL, "vsnprintf(): Unable to concatinate string.");

            s += 2;
            pad = '0';

            width = (flags & FLAGS_MAXINT) ? 16 : 8;

            /* FALL THROUGH */

        case 'X':
            flags |= FLAGS_UPPER;
hex:
            flags |= FLAGS_NOSIGN;
            base = 16;

number:
            if ((flags & (FLAGS_MAXINT | FLAGS_LONGLONG)) != 0)
                val = va_arg(args, long long);
            else if ((flags & FLAGS_LONGINT) != 0)
                val = va_arg(args, long);
            else if ((flags & FLAGS_SHORT) != 0)
                val = va_arg(args, int);
            else
                val = va_arg(args, int32_t);

            if (flags & FLAGS_NOSIGN)
                sign = '\0';
            else
            {
                if (val < 0)
                {
                    sign = '-';
                    val = -val;
                }
            }

            p = base_to_ascii(val, base, (flags & FLAGS_UPPER) != 0, nbuf, sizeof(nbuf));

            n = sizeof(nbuf) - (p - nbuf + 1);

            if (sign != '\0')
                sbuf[s++] = sign;

            goto string;

        case 's':
            p = va_arg(args, char *);
            n = strlen(p);
            pad = ' ';

string:
            if (width > (slen - s - 1))
                width = slen - s - 1;

            if (width > n)
            {
                // We need to add padding....
                memset(sbuf + s, pad, (width - n));
                s += (width - n);
            }

            sbuf[s] = '\0';
            p = _s_strncat(sbuf + s, (slen - s), p, n);

            ASSERT(p != NULL, "vsnprintf(): Unable to concatinate string.");

            s += n;

            break;
        }

        ++fp;
    }

exit:
    sbuf[s] = '\0';
    return s;
}

/********************************************************************************************************************/
