/*************************************************************************/
/*
 * $Id: kheap.h 44 2010-02-19 23:53:25Z kfiresun $
 */
/*************************************************************************/

#ifndef _FOO_KHEAP_H__
#define _FOO_KHEAP_H__

/*************************************************************************/

#define KHEAP_START		0xC0000000
#define KHEAP_INITIAL_SIZE	0x00100000
#define KHEAP_MAX_SIZE		0xCFFFF000

/*************************************************************************/

typedef struct heap_TYPE heap_t;

/*************************************************************************/

void *KMemAlloc(size_t sz, bool_t align, void **phys);
void kfree(void *);

void *kmalloc_ap(size_t sz, void **phys);
void *kmalloc_a(size_t sz);
void *kmalloc_p(size_t sz, void **phys);
void *kmalloc(size_t sz);

heap_t *create_heap(
	uintptr_t start,
	uintptr_t end,
	size_t max,
	bool_t supervisor,
	bool_t readonly);

/*************************************************************************/

#endif /* _FOO_KHEAP_H__ */

/*************************************************************************/
