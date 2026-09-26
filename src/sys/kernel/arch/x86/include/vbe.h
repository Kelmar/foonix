/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef __FOONIX_ARCH_x86_VBE_H__
#define __FOONIX_ARCH_x86_VBE_H__

/********************************************************************************************************************/

namespace vesa
{

enum class VideoCaps : uint32_t
{
    /// @brief DAC can be switched into 8-bit indexed mode.
    DAC          = 1 << 0;

    /// @brief non-VGA controller.
    NonVGA       = 1 << 1;

    /// @brief DAC supports blanking bit.
    BlankDAC     = 1 << 2;
    
    /// @brief Stereoscopic display signaling
    Stereoscopic = 1 << 3;

    /// @brief VBE 3.0 extensions
    Extensions   = 1 << 4;

    /// @brief Hardware mouse cursor
    HWMouse      = 1 << 5;

    /// @brief Hardware clipping
    HWClipping   = 1 << 6;

    /// @brief Hardware transparent BitBLT
    TransBitBLT  = 1 << 7;

    // Bits 8-31 reserved.
};

/********************************************************************************************************************/

enum class MemoryModel : uint8_t
{
    Text        = 0,
    CGAGraphics = 1, // 4-color?
    HGCGraphics = 2, // 2-color?
    EGAGraphics = 3, // 16-color mode
    PackedPixel = 4,
    NonChain    = 5, // Unclear (8-bit indexed?)
    DirectColor = 6, // 15, 16, and 24 bit RGB color modes
    YUVColor    = 7  // Luminance Chrominance Value

    // 0x08 ~ 0x0F reserved for VESA
    // 0x10 ~ 0xFF reserved for OEMs
};

/********************************************************************************************************************/

enum class ModeAttributes : uint16_t
{
    Supported    = 1 << 0,
    OptionalInfo = 1 << 1,
    BIOSSupport  = 1 << 2,
    IsColor      = 1 << 3,
    IsGraphics   = 1 << 4,
    
    // VBE 2.0
    NonVGA       = 1 << 5,
    Banked       = 1 << 6,
    Linear       = 1 << 7,
    DoubleScan   = 1 << 8, // E.g. 320x200 and 320x240

    // VBE 3.0
    Interlaced   = 1 << 9,
    TripleBuffer = 1 << 10,
    Stereoscopic = 1 << 11,
    DualDisplay  = 1 << 12,

    // 13 - 15 reserved
};

/********************************************************************************************************************/

enum class WindowAttributes : uint8_t
{
    Exists = 1 << 0,
    Readable = 1 << 1,
    Writable = 1 << 2

    // 3 - 7 are reserved
};

/********************************************************************************************************************/
/**
 * @brief VESA video information block.
 *
 * @remarks Info from INT 0x10, function 0x4F00
 */
struct VideoInfo
{
    uint8_t   signature[4]; // "VESA" or "VBE2"
    uint8_t   version_major;
    uint8_t   version_minor;
    uint32_t  oem_name_ptr;
    VideoCaps caps;
    uint32_t  supported_modes_ptr; // List of 16-bit words terminated with 0xFFFF
    uint16_t  memory_size_blocks;  // 64KB blocks

    // VBE 2.0 and higher only
    uint8_t   oem_software_major_version; // BCD
    uint8_t   oem_software_minor_version; // BCD
    uint32_t  vendor_name_ptr;
    uint32_t  product_name_ptr;
    uint32_t  product_revision_str_ptr;
    uint16_t  vbe_af_version;    // If cap flags bit 3 set (BCD)
    uint32_t  acc_mode_list_ptr; // If cap flags bit 3 set

    uint8_t   reserved[216];

    uint8_t   oem_scratch_pad[256]; // OEM specific data
} __attribute__((packed));

/********************************************************************************************************************/
/**
 * @brief 256 byte structure of VBE mode information details.
 *
 * @remarks Info from INT 0x10, function 0x4F01
 */
struct ModeInfo
{
    ModeAttributes   attributes;
    WindowAttributes window_attributes_a;
    WindowAttributes window_attributes_b;

    uint16_t  window_granularity;   // In KB
    uint16_t  window_size;          // In KB
    uint16_t  window_a_start;       // Segment (0 if not supported)
    uint16_t  window_b_start;       // Segment (0 if not supported)
    uint32_t  window_position_ptr;  // Pointer to a function
    uint16_t  pitch;                // In bytes

    // Optional for VESA 1.0 and 1.1
    uint16_t  width;                // In pixels
    uint16_t  height;               // In pixels
    uint8_t   char_width;           // In pixels
    uint8_t   char_height;          // In pixels

    /// @brief Number of memory planes
    uint8_t   planes;
    uint8_t   depth;                // In bits per pixel

    /// @brief Number of memory banks
    uint8_t   banks;
    MemoryModel memory_model;
    uint8_t   bank_size;            // In KB
    uint8_t   image_pages;
    uint8_t   reserved0;            // 0 for VBE 1.0-2.0, 1 for VBE 3.0

    // VBE 1.2
    uint8_t  red_mask_size;         // In bits
    uint8_t  red_mask_start;        // Bit position
    uint8_t  green_mask_size;       // In bits
    uint8_t  green_mask_start;      // Bit position
    uint8_t  blue_mask_size;        // In bits
    uint8_t  blue_mask_start;       // Bit position

    uint8_t  reserved_mask_size;    // In bits
    uint8_t  reserved_mask_start;   // Bit position

    /*
     * Bit 0: Color ramp is programmable.
     * Bit 1: Bytes in reserved field may be used by application.
     */
    uint8_t  direct_color_attributes;

    // VBE 2.0
    uint32_t  framebuffer_start;    // Framebuffer physical address
    uint32_t  off_screen_offset;    // Pointer to off screen memory
    uint16_t  off_screen_size;      // Size of off screen memory in KB

    // VBE 3.0
    uint16_t  framebuffer_pitch;    // In bytes
    uint8_t   banked_images;        // Number of images (less one) for banked video modes
    uint8_t   linear_images;        // Number of images (less one) for linear video modes

    // Linear framebuffer bit mask values.  (Are these redundant?)
    uint8_t   red_mask_size2;
    uint8_t   red_mask_start2;
    uint8_t   green_mask_size2;
    uint8_t   green_mask_start2;
    uint8_t   blue_mask_size2;
    uint8_t   blue_mask_start2;
    uint8_t   color_mask_size;
    uint8_t   color_mask_start;

    uint32_t  max_dot_clock;        // In Hz

    uint8_t   reserved1[190];       // Not always zeroed out.
} __attribute__((packed));

}

/********************************************************************************************************************/

#endif /* __FOONIX_ARCH_x86_VBE_H__ */

/********************************************************************************************************************/
