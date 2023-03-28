/*************************************************************************/
/*************************************************************************/

#ifndef __FOONIX_ARCH_I386_VGA_H__
#define __FOONIX_ARCH_I386_VGA_H__

#include <stdint.h>

/*************************************************************************/

enum vga_color
{
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN   = 14,
    VGA_COLOR_WHITE         = 15
};

static inline uint8_t vga_entry_color(enum vga_color foreground, enum vga_color background)
{
    return (background << 4) | foreground;
}
 
static inline uint16_t vga_entry(char c, uint8_t color)
{
    return (uint16_t)c | (uint16_t)(color << 8);
}

/*************************************************************************/

#endif /* __FOONIX_ARCH_I386_VGA_H__ */

/*************************************************************************/
