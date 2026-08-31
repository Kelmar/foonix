#include <stdint.h>
#include <stddef.h>
#include <sys/cdefs.h>

#define ERROR(X_) kpanic(X_)

#include <kassert.h>


extern "C" void __cxa_pure_virtual(void)
{
    ERROR("Pure virtual call in kernel");
}

extern "C" int _purecall()
{
    ERROR("Pure call in kernel");
    return 0;
}

extern "C" void __cxa_atexit()
{
}
