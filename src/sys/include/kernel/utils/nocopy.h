/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_KERNEL_UTILS_NOCOPY_H__
#define __FOONIX_KERNEL_UTILS_NOCOPY_H__

/********************************************************************************************************************/

namespace util
{
    class nocopy
    {
    private:
        nocopy(const nocopy &) = delete;
        nocopy &operator =(const nocopy &) = delete;

    protected:
        constexpr nocopy() noexcept = default;
        ~nocopy() { }
    };

    class nomove
    {
    private:
        nomove(nomove &&) = delete;
        nomove &operator =(nomove &&) = delete;

    protected:
        constexpr nomove() noexcept = default;
        ~nomove() { }
    };
}

/********************************************************************************************************************/

#endif /* __FOONIX_KERNEL_UTILS_NOCOPY_H__ */

/********************************************************************************************************************/
