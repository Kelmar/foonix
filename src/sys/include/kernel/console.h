/********************************************************************************************************************/

#ifndef __FONIX_KERNEL_CONSOLE_H__
#define __FONIX_KERNEL_CONSOLE_H__

/********************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

#include <concepts>
#include <string_view>

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
    void putstr(const std::string_view &v);

    void putuint(uint64_t value, int radix = 10, int width = 0);
    void putint(int64_t value);
    
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

inline
Console &operator <<(Console &cons, uint64_t value)
{
    cons.putuint(value);
    return cons;
}

inline
Console &operator <<(Console &cons, int64_t value)
{
    cons.putint(value);
    return cons;
}

inline Console &operator <<(Console &cons, uint32_t value) { return operator <<(cons, static_cast<uint64_t>(value)); }
inline Console &operator <<(Console &cons,  int32_t value) { return operator <<(cons, static_cast<int64_t> (value)); }

inline Console &operator <<(Console &cons, uint16_t value) { return operator <<(cons, static_cast<uint64_t>(value)); }
inline Console &operator <<(Console &cons,  int16_t value) { return operator <<(cons, static_cast<int64_t> (value)); }

inline Console &operator <<(Console &cons, uint8_t value) { return operator <<(cons, static_cast<uint64_t>(value)); }
inline Console &operator <<(Console &cons,  int8_t value) { return operator <<(cons, static_cast<int64_t> (value)); }

inline
Console &operator <<(Console &cons, const std::string_view &str)
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

struct hex
{
    unsigned int value;
    int width;

    hex(unsigned int v, int w = 0) : value(v), width(w) { }

    Console &to_stream(Console &c) const
    {
        c.putuint(value, 16, width);
        return c;
    }
};

/********************************************************************************************************************/

#endif /* __FONIX_KERNEL_CONSOLE_H__ */

/********************************************************************************************************************/
