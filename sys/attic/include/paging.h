/*************************************************************************/
/*
 * $Id: paging.h 63 2010-09-03 01:25:50Z kfiresun $ 
 */
/*************************************************************************/

#ifndef _FOO_PAGING_H__
#define _FOO_PAGING_H__

/*************************************************************************/

#include "cdefs.h"
#include "stdint.h"
#include "multboot.h"

/*************************************************************************/

#define PAGE_FLAG_UNMOVABLE 0x01    /* Don't swap this page out to disk */

/*************************************************************************/

typedef struct page_TYPE page_t;
typedef struct page_directory_TYPE page_directory_t;

/*************************************************************************/

/*
 * Initializes paging.
 */
void init_paging(multiboot_t *mbd);

#define AFF_KERNEL 0x00000000 /* Page is for kernel use only */
#define AFF_USER   0x00000001 /* Page is for kernel and user use */
#define AFF_RDONLY 0x00000000 /* Page is read only */
#define AFF_RDWR   0x00000002 /* Page is readable and writable */

void alloc_frame(page_t *page, int flags);
void free_frame(page_t *page);

page_t *get_page(uintptr_t address, bool_t create, page_directory_t *dir);

/*************************************************************************/

#endif /* _FOO_PAGING_H__ */

/*************************************************************************/
