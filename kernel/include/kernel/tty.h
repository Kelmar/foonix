/*************************************************************************/

#ifndef __FOONIX_TTY_H__
#define __FOONIX_TTY_H__

#include <sys/cdefs.h>

#include <stddef.h>

/*************************************************************************/

__BEGIN_EXTERN_C

void terminal_init(void);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_write(const char *data, size_t size);
void terminal_writestr(const char *data);

__END_EXTERN_C

/*************************************************************************/

#endif /* __FOONIX_TTY_H__ */

/*************************************************************************/
