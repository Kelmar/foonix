/********************************************************************************************************************/
/********************************************************************************************************************/

#include <kassert.h>

#include <kernel/kalloc.h>

/********************************************************************************************************************/

void* operator new(size_t sz)
{
    void* rval = kalloc(sz);

    if (rval == nullptr)
        kpanic("Out of memory");

    return rval;
}

void* operator new[](size_t sz)
{
    void* rval = kalloc(sz);

    if (rval == nullptr)
        kpanic("Out of memory");

    return rval;
}

/********************************************************************************************************************/

void operator delete(void* ptr) throw()
{
    kfree(ptr);
}

void operator delete[](void* ptr) throw()
{
    kfree(ptr);
}

/********************************************************************************************************************/

void operator delete(void* ptr, size_t) throw()
{
    kfree(ptr);
}

void operator delete[](void* ptr, size_t) throw()
{
    kfree(ptr);
}

/********************************************************************************************************************/
