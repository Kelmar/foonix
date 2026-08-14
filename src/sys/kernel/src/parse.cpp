/********************************************************************************************************************/

#include <ctype.h>
#include <string.h>

#include <expected>
#include <string_view>

#include <kernel/kernel.h>
#include <kernel/utilities.h>

/********************************************************************************************************************/

std::expected<int, Kernel::ErrorCode> util::parseInt(const std::string_view &str)
{
    if (str.empty())
        return Kernel::ErrorCode::NotFound;

    size_t max = str.length();

    bool isHex = max > 2 && (str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X'));
    size_t index = 0;
    int acc = 0;

    if (isHex)
        index += 2;
    else if (str[0] == '$')
    {
        isHex = true;
        ++index;
    }

    for (; index < max; ++index)
    {
        int a = str[index];

        if (isdigit(a))
            a -= '0';
        else if (isHex && (a >= 'A') && (a <= 'F'))
            a -= ('A' - 10);
        else if (isHex && (a >= 'a') && (a <= 'f'))
            a -= ('a' - 10);
        else
            return Kernel::ErrorCode::InvalidFormat;

        acc *= isHex ? 16 : 10;
        acc += a;
    }

    return acc;
}

/********************************************************************************************************************/


