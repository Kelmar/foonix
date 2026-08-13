/********************************************************************************************************************/

#ifndef __FONIX_KERNEL_CONSOLE_H__
#define __FONIX_KERNEL_CONSOLE_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <concepts>

#include <kernel/span.h>

/********************************************************************************************************************/

class Console
{
private:
    Console(const Console &) = delete;
    Console(Console &&) = delete;

private:
    bool m_paged;
    int m_lineCount;

    void pause_check();

public:
    /* constructor */ Console() noexcept
        : m_paged(false)
        , m_lineCount(0)
    {
    }

    ~Console() noexcept { }

    void init();
    
    // Output
    bool set_paged(bool value);

    void putchar(char c);
    void putstr(const char *str);

    // Input
    int getchar();

    size_t get_line(util::span<char> &buf);

    inline
    size_t get_line(char *buf, size_t size)
    {
        util::span<char> data(buf, size);
        return get_line(data);
    }
};

extern Console console;

/********************************************************************************************************************/
// Operators for outputing a basic types.

inline
Console &operator <<(Console &cons, const char *str)
{
    cons.putstr(str);
    return cons;
}

/********************************************************************************************************************/
// Operator for outputing classes that implement to_stream() method.

template <typename T>
concept serializable = requires(T a, Console c)
{
    { a.to_stream(c) } -> std::same_as<Console &>;
};

template <serializable T>
inline 
Console &operator <<(Console &cons, const T &obj)
{
    return obj.to_stream(cons);
}

/********************************************************************************************************************/

#endif /* __FONIX_KERNEL_CONSOLE_H__ */

/********************************************************************************************************************/
