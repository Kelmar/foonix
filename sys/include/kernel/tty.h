/********************************************************************************************************************/

#ifndef __FOONIX_TTY_H__
#define __FOONIX_TTY_H__

#include <sys/cdefs.h>

#include <stddef.h>
#include <stdint.h>

/********************************************************************************************************************/

__BEGIN_EXTERN_C

void terminal_init(void);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_write(const char *data, size_t size);
void terminal_writestr(const char *data);

void terminal_write64(uint64_t val);

__END_EXTERN_C

/********************************************************************************************************************/

#endif /* __FOONIX_TTY_H__ */

/********************************************************************************************************************/
