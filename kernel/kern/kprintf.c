/*************************************************************************/
/*
 * $Id: kprintf.c 62 2010-07-08 01:30:35Z kfiresun $ 
 */
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "stdarg.h"
#include "stdlib.h"
#include "kernio.h"

/*************************************************************************/
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
 */

/*************************************************************************/
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
char *base_to_ascii(uint32_t val, int base, bool_t upper,
		    char *buf, size_t buflen)
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

/*************************************************************************/

#define FLAGS_UPPER	0x0001
#define FLAGS_NOSIGN	0x0002

/*************************************************************************/
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
 * Formatting options include:
 * - Padding with optional leading zeros for numbers.
 * - Sign extending for signed number values.
 *
 * Loosly based on the BSD vsnprintf() function.
 */
int vsnkprintf(char *sbuf, size_t slen, const char *fmt, va_list args)
{
    const char *fp = fmt;
    size_t base, width;
    char nbuf[32], *p;
    size_t s = 0, n;
    char pad, sign;
    int flags, val;

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

        case 'c':
            sbuf[s++] = (char)va_arg(args, int);
            break;

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
            p = strkcat(sbuf + s, (slen - s), "0x", 3);
            if (p == NULL)
                kstop("vsnkprintf(): unable to concatinate string.");

            s += 2;
            pad = '0';

#if defined(_FOO64)
            width = 16;
#else
            width = 8;
#endif

            /* FALL THROUGH */

        case 'X':
            flags |= FLAGS_UPPER;
hex:
            flags |= FLAGS_NOSIGN;
            base = 16;

number:
            val = va_arg(args, int);

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

            p = base_to_ascii((uint32_t)val, base, (flags & FLAGS_UPPER) != 0, nbuf, sizeof(nbuf));

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
            p = strkcat(sbuf + s, (slen - s), p, n);
            if (p == NULL)
            kstop("vsnkprintf(): unable to concatinate string.");

            s += n;

            break;
        }

        ++fp;
    }

exit:
    sbuf[s] = '\0';
    return s;
}

/*************************************************************************/

int snkprintf(char *sbuf, size_t slen, const char *fmt, ...)
{
    int rval = 0;
    va_list args;

    va_start(args, fmt);
    rval = vsnkprintf(sbuf, slen, fmt, args);
    va_end(args);

    return rval;
}

/*************************************************************************/

int vkprintf(const char *fmt, va_list args)
{
    char buf[1024];
    int rval = 0;

    rval = vsnkprintf(buf, sizeof(buf), fmt, args);
    putstr(buf);

    return rval;
}

/*************************************************************************/

int kprintf(const char *fmt, ...)
{
    va_list args;
    int rval;

    va_start(args, fmt);
    rval = vkprintf(fmt, args);
    va_end(args);

    return rval;
}

/*************************************************************************/
/*************************************************************************/

void kprint_bitinfo(bitinfo_t *info, uint32_t bits)
{
    uint32_t u = bits;
    int i = 0;

    while (info[i].bit != 0)
    {
        if ((u & info[i].bit) == info[i].bit)
        {
            putstr(info[i].name);

            u &= ~info[i].bit;
            if (u)
                putstr(" ");
        }

        ++i;
    }
}

/*************************************************************************/
