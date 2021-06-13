#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// TODO: This stuff should be moved into a libc++

#ifdef __is_libk
# define THROW_SIG throw()
# define ERROR(X_) panic(X_)

# include <kernel/flow.h>

#else
# define THROW_SIG
# define ERROR(X_) throw std::exception(X_)

# include <stdexcept>

#endif

void* operator new(size_t sz) THROW_SIG
{
    void* rval = malloc(sz);

    if (rval == nullptr)
    {
        ERROR("Out of memory");
    }

    return rval;
}

void* operator new[](size_t sz) THROW_SIG
{
    void* rval = malloc(sz);

    if (rval == nullptr)
    {
        ERROR("Out of memory");
    }

    return rval;
}

void operator delete(void* ptr) throw()
{
    free(ptr);
}

void operator delete[](void* ptr) throw()
{
    free(ptr);
}

void operator delete(void* ptr, size_t) throw()
{
    free(ptr);
}

void operator delete[](void* ptr, size_t) throw()
{
    free(ptr);
}

extern "C" void __cxa_pure_virtual(void)
{
#ifdef __is_libk
    panic("Pure virtual call in kernel!");
#endif
}

extern "C" int _purecall()
{
#ifdef __is_libk
    panic("Pure call in kernel!");
#endif

    return 0;
}
