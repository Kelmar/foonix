/*************************************************************************/
/*
 * $Id: klist.h 44 2010-02-19 23:53:25Z kfiresun $
 */
/*************************************************************************/

#ifndef _FOO_KLIST_H__
#define _FOO_KLIST_H__

/*************************************************************************/

typedef int (*kcomp_func_t)(void *, void *);

typedef void *(*kalloc_func_t)(size_t);
typedef void  (*kfree_func_t)(void *);

struct kallocator_TYPE
{
    kalloc_func_t alloc;
    kfree_func_t  free;
    void	 *user;
};

typedef struct kallocator_TYPE kallocator_t;

struct klist_TYPE;
typedef struct klist_TYPE klist_t;

/*************************************************************************/

klist_t *klist_create(size_t max_size, kcomp_func_t, const kallocator_t *);
void klist_delete(klist_t *);

void klist_insert(klist_t *, void *);
void klist_remove(klist_t *, size_t index);
void *klist_get_at(klist_t *, size_t index);

size_t klist_size(klist_t *);
size_t klist_count(klist_t *);
size_t klist_max_count(klist_t *);

/*************************************************************************/

#endif /* _FOO_KLIST_H__ */

/*************************************************************************/
