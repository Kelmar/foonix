/*************************************************************************/
/*
 * $Id: klist.c 187 2011-08-27 16:27:33Z kfiresun $
 */
/*************************************************************************/
/*
 * A pretty brain dead ordered list.
 *
 * Limitations:
 * - It's slow
 * - It can't expand
 * - It's slow
 * - Kinda kludgy memory usage.
 * - Did I mention that it's slow?
 *
 * Lookup time is o(1)
 * Insert time is o(n)	-- Ew!
 * Removal time is o(n)	-- Ew!
 */
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "stdarg.h"
#include "stdlib.h"
#include "kernio.h"
#include "kheap.h"
#include "klist.h"

/*************************************************************************/

struct klist_TYPE
{
    size_t	 count;		// Number of items in array
    void       **array;		// Item array

    size_t	 max_count;	// Maximum number of elements
    size_t	 size;		// Total allocated size in bytes
    kcomp_func_t cmp_func;	// Compare function

    kalloc_func_t alloc;	// Allocation function
    kfree_func_t  free;		// Free function
    void         *user;		// User data
};

/*************************************************************************/
/**
 * Creates a new empty list at the specified memory location.
 */
klist_t *klist_create_direct(
		      size_t max_count,
		      kcomp_func_t cmp_func,
		      void *start,
		      const kallocator_t *alloc)
{
    size_t array_sz = sizeof(void *) * max_count;
    size_t list_sz = sizeof(klist_t);
    klist_t *rval = (klist_t *)start;

    memset(rval, 0, list_sz);

    rval->alloc = alloc ? alloc->alloc : kmalloc;
    rval->free = alloc ? alloc->free : kfree;

    rval->max_count = max_count;
    rval->cmp_func = cmp_func;

    rval->array = (void **)rval->alloc(array_sz);
    memset(rval->array, 0, array_sz);

    rval->size = list_sz + array_sz;
    rval->count = 0;

    return rval;
}

/*************************************************************************/
/**
 * Creates a new empty list.
 */
klist_t *klist_create(size_t max_count,
		      kcomp_func_t cmp_func,
		      const kallocator_t *alloc)
{
    size_t list_sz = sizeof(klist_t);
    void *ptr;

    if (alloc == NULL)
    {
	// Use default allocation method.
        ptr = kmalloc(list_sz);
    }
    else
        ptr = alloc->alloc(list_sz);

    return klist_create_direct(max_count, cmp_func, ptr, alloc);
}

/*************************************************************************/
/**
 * Disposes of a list of items.
 */
void klist_free(klist_t *list)
{
    if (!list || (!list->free))
	return; // Can't free

    list->free(list->array);
    list->free(list);
}

/*************************************************************************/
/**
 * Inserts an item into our list.  Items are ordered based on the compare
 * function that was suppiled when the list was created.
 */
void klist_insert(klist_t *list, void *item)
{
    register size_t i = 0;

    ASSERT(list != NULL, "Invalid list");
    ASSERT(list->count < list->max_count, "List overflow");

    if (!list)
	return;

    while ((i < list->count) && (list->cmp_func(list->array[i], item) < 0))
	++i;

    if (i < list->count)
    {
        // Create a gap for our new item.
	register size_t j = list->count;

	while (j >= i)
	{
	    list->array[j] = list->array[j - 1];
	    --j;
	}
    }

    list->array[i] = item;
    ++list->count;
}

/*************************************************************************/
/**
 * Removes an item at the specified point in our list.
 */
void klist_remove(klist_t *list, size_t index)
{
    ASSERT(list != NULL, "Invalid list");
    ASSERT(index < list->size, "Invalid index");

    if (index < (list->count - 1))
    {
	size_t cnt = list->count - index;

	memcpy(
	    list->array[index],
	    list->array[index + 1],
	    sizeof(void *) * cnt);
    }

    list->array[--list->count] = NULL;
}

/*************************************************************************/
/**
 * Gets an item located at a specific point in our list.
 */
void *klist_get_at(klist_t *list, size_t index)
{
    ASSERT(list != NULL, "Invalid list");
    ASSERT(index < list->count, "Invalid index");

    return list->array[index];
}

/*************************************************************************/
/**
 * Return the size of the list in bytes.
 */
size_t klist_size(klist_t *list)
{
    ASSERT(list != NULL, "Invalid list");

    return list->size;
}

/*************************************************************************/
/**
 * Gets the number of items inserted into the list.
 */
size_t klist_count(klist_t *list)
{
    ASSERT(list != NULL, "Invalid list");

    return list->count;
}

/*************************************************************************/
/**
 * Gets the maximum number of items that can fit into the list.
 */
size_t klist_max_count(klist_t *list)
{
    ASSERT(list != NULL, "Invalid list");

    return list->max_count;
}

/*************************************************************************/
