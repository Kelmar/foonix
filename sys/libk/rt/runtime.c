/********************************************************************************************************************/
/*
 * Generic runtime initialization/destruction functions.
 */
/********************************************************************************************************************/

#include <stddef.h>

/********************************************************************************************************************/

#define IN_SECTION(X_) __attribute__((section(X_), visibility("hidden"), used))
#define USED __attribute__((used))
#define WEAK __attribute__((weak))

#define EXPECT(X_, Y_) __builtin_expect(X_, Y_)

/********************************************************************************************************************/
// Prototype for constructors and destructors.
typedef void (*fn)(void);

// These are defined by the compiler.

fn __CTOR_LIST__    [] IN_SECTION(".ctors") = { 0 };
fn __CTOR_LIST_END__[] IN_SECTION(".ctors") = { 0 };

fn __DTOR_LIST__    [] IN_SECTION(".dtors") = { 0 };
fn __DTOR_LIST_END__[] IN_SECTION(".dtors") = { 0 };

/********************************************************************************************************************/

USED
void __runtime_init()
{
    static int called = 0; // Guard against acidental second call.

    if (EXPECT(called, 0))
        return;

    /*
     * According to the GCC manual, the first item in the .ctor list is 0, 1 or length of 
     * the array depending on the platform.
     *
     * We're going to just ignore it and use __CTOR_LIST_END__ to get our size.
     */
    const size_t CNT = __CTOR_LIST_END__ - __CTOR_LIST__ - 1;

    // Call the constructors in reverse order
    for (size_t i = CNT; i >= 1; --i)
        __CTOR_LIST__[i]();
}

/********************************************************************************************************************/

USED
void __runtime_fini()
{
    static int called = 0; // Guard against acidental second call.

    if (EXPECT(called, 0))
        return;

    // Same as constructors, skip first
    const size_t CNT = __DTOR_LIST_END__ - __DTOR_LIST__ - 1;

    // Unlike constructors, we call in forward order.
    for (size_t i = 1; i <= CNT; ++i)
        __DTOR_LIST__[i]();
}

/********************************************************************************************************************/
