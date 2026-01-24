#include <stdint.h>
#include <stddef.h>

//#define THROW_SIG throw()
#define THROW_SIG 
#define ERROR(X_) Debug::Panic(X_)

#include <kernel/debug.h>
#include <kernel/flow.h>
#include <kernel/vm/vm.h>

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
    kpanic("Pure virtual call in kernel!");
}

extern "C" int _purecall()
{
    kpanic("Pure call in kernel!");
    return 0;
}

extern "C" void __cxa_atexit()
{
}
