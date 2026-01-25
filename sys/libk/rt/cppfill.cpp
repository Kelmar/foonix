#include <stdint.h>
#include <stddef.h>

//#define THROW_SIG throw()
#define THROW_SIG 
#define ERROR(X_) kpanic(X_)

#include <kernel/debug.h>
#include <kernel/flow.h>
#include <kernel/vm.h>

void* operator new(size_t sz) THROW_SIG
{
    UNUSED(sz);
    void* rval = nullptr; //malloc(sz);

    if (rval == nullptr)
        kpanic("Out of memory");

    return rval;
}

void* operator new[](size_t sz) THROW_SIG
{
    UNUSED(sz);
    void* rval = nullptr; //malloc(sz);

    if (rval == nullptr)
        kpanic("Out of memory");

    return rval;
}

void operator delete(void* ptr) throw()
{
    UNUSED(ptr);
    //free(ptr);
}

void operator delete[](void* ptr) throw()
{
    UNUSED(ptr);
    //free(ptr);
}

void operator delete(void* ptr, size_t) throw()
{
    UNUSED(ptr);
    //free(ptr);
}

void operator delete[](void* ptr, size_t) throw()
{
    UNUSED(ptr);
    //free(ptr);
}

extern "C" void __cxa_pure_virtual(void)
{
    ERROR("Pure virtual call in kernel!");
}

extern "C" int _purecall()
{
    ERROR("Pure call in kernel!");
    return 0;
}

extern "C" void __cxa_atexit()
{
}
