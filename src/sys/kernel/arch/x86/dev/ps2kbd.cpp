/********************************************************************************************************************/
/*
 * Driver for PS/2 keyboard interface.
 */
/********************************************************************************************************************/

#include <stdint.h>
#include <string.h>

#include <kernel/utilities.h>

#include <kernel/tty.h>

//#include "bus.h"
//#include "ps2kbd.h"
//#include "memory.h"
#include "cpu.h"

/********************************************************************************************************************/

#define DATA_PORT 0x0060
#define STATUS_PORT 0x0064

#define STATUS_READ 0x01

/********************************************************************************************************************/

namespace
{
    /// @brief Flags for processing shift and lock keys.
    enum class ShiftFlags 
    {
        None   = 0x0000, // No shift keys pressed
        Shift  = 0x0001, // Shift is pressed
        Ctrl   = 0x0002, // CTRL is pressed
        Alt    = 0x0004, // ALT is pressed
        Cmd    = 0x0008, // "Command" is pressed (i.e. Logo key)
        Caps   = 0x0100, // Caps lock on
        Num    = 0x0200, // Num lock on
        Scroll = 0x0400, // Scroll lock on
        Extend = 0x8000  // Extended keycode received (removed after next scan code received)
    };
}

as_flags(ShiftFlags);

namespace
{
    /************************************************************************************************************/

    // Scan Code Table
    //     0  , 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
    // 0 - err, esc, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, -, =, bksp, tab,
    // 1 - q, w, e, r, t, y, u, i, o, p, [, ], enter, lctrl, a, s
    // 2 - d, f, g, h, j, k, l, ;, ', `, lshift, \, z, x, c, v,
    // 3 - b, n, m, ,, ., /, rshift, *p, lalt, space, caps, f1, f2, f3, f4, f5,
    // 4 - f6, f7, f8, f9, f10, numlock, scroll, 7p, 8p, 9p, -p, 4p, 5p, 6p, +p, 1p,
    // 5 - 2p, 3p, 0p, .p, SysRq, ?, ?, f11, f12
    //
    // Legend
    //   p - suffix num pad key
    // 

    /// @brief Action to be taken for a given scan code.
    enum class KeyAction : uint8_t
    {
        /// @brief Key is a regular key
        Regular,

        /// @brief Key is a num pad key
        Pad,

        /// @brief Key is a control key (Esc, F1, etc)
        Control,

        /// @brief Key is a shift key (Shift, CTRL, Alt, etc)
        Shift,

        /// @brief Key is a lock key (Caps, Num, Scroll, etc)
        Lock,

        /// @brief Start of extended key code.
        Extend,

        /// @brief Key should be ignored
        Ignore
    };

    /************************************************************************************************************/

    /// @brief Lookup table of scan code to actions.
    KeyAction s_actions[256];
    
    /// @brief Initializes the actions table.
    void InitActionsTable()
    {
        for (int i = 1; i < 256; ++i)
            s_actions[i] = (i & 0x80) ? KeyAction::Ignore : KeyAction::Regular;

        s_actions[0x00] = KeyAction::Ignore;  // Error code
        s_actions[0x1D] = KeyAction::Shift;   // Left CTRL
        s_actions[0x2A] = KeyAction::Shift;   // Left SHIFT
        s_actions[0x36] = KeyAction::Shift;   // Right SHIFT
        s_actions[0x38] = KeyAction::Shift;   // Left ALT
        s_actions[0x3A] = KeyAction::Lock;    // Caps Lock
        s_actions[0x3B] = KeyAction::Control; // F1
        s_actions[0x3C] = KeyAction::Control; // F2
        s_actions[0x3D] = KeyAction::Control; // F3
        s_actions[0x3E] = KeyAction::Control; // F4
        s_actions[0x3F] = KeyAction::Control; // F5
        s_actions[0x40] = KeyAction::Control; // F6
        s_actions[0x41] = KeyAction::Control; // F7
        s_actions[0x42] = KeyAction::Control; // F8
        s_actions[0x43] = KeyAction::Control; // F9
        s_actions[0x44] = KeyAction::Control; // F10
        s_actions[0x45] = KeyAction::Lock;    // Num lock
        s_actions[0x46] = KeyAction::Lock;    // Scroll lock
        s_actions[0x47] = KeyAction::Pad;     // 7 (Home)
        s_actions[0x48] = KeyAction::Pad;     // 8 (Up Arrow)
        s_actions[0x49] = KeyAction::Pad;     // 9 (Page up)
        s_actions[0x4A] = KeyAction::Pad;     // - (num pad)
        s_actions[0x4B] = KeyAction::Pad;     // 4 (Left Arrow)
        s_actions[0x4C] = KeyAction::Pad;     // 5 ()
        s_actions[0x4D] = KeyAction::Pad;     // 6 (Right Arrow)
        s_actions[0x4E] = KeyAction::Pad;     // +
        s_actions[0x4F] = KeyAction::Pad;     // 1 (End)
        s_actions[0x50] = KeyAction::Pad;     // 2 (Down Arrow)
        s_actions[0x51] = KeyAction::Pad;     // 3 (Page Down)
        s_actions[0x52] = KeyAction::Pad;     // 0 (Insert)
        s_actions[0x53] = KeyAction::Pad;     // . (Delete)
        s_actions[0x54] = KeyAction::Control; // Print Screen/SysRq
        s_actions[0x57] = KeyAction::Control; // F11
        s_actions[0x58] = KeyAction::Control; // F12

        s_actions[0xE0] = KeyAction::Extend;
        s_actions[0xE1] = KeyAction::Extend;
    }

    /************************************************************************************************************/

    /// @brief Holds current shift state
    ShiftFlags s_shift = ShiftFlags::None;

    /// @brief Last scan code processed.
    uint8_t s_lastCode;

    /// @brief Scan code to character map when not shifted.
    char s_normalMap[256] =
    {
    //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
        0x00, 0x1B, '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , 0x08, 0x09, // 0x00
        'q' , 'w' , 'e' , 'r' , 't' , 'y' , 'u' , 'i' , 'o' , 'p' , '[' , ']' , '\n', 0x00, 'a' , 's',  // 0x10
        'd' , 'f' , 'g' , 'h' , 'j' , 'k' , 'l' , ';' , '\'', '`' , 0x00, '\\', 'z' , 'x' , 'c' , 'v',  // 0x20
        'b' , 'n' , 'm' , ',' , '.' , '/' , 0x00, '*' , 0x00, ' ' , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x30
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '7' , '8' , '9' , '-' , '4' , '5' , '6' , '+' , '1',  // 0x40
        '2' , '3' , '0' , '.' , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x50
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x60
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x70
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\n', 0x00, 0x00, 0x00, // 0x90
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xA0
        0x00, 0x00, 0x00, 0x00, 0x00, '/' , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xC0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xD0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xE0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 0xF0
    };

    /// @brief Scan code to character map when shifted.
    char s_shiftMap[256] =
    {
    //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
        0x00, 0x1B, '!' , '@' , '#' , '$' , '%' , '^' , '&' , '*' , '(' , ')' , '_' , '+' , 0x08, 0x09, // 0x00
        'Q' , 'W' , 'E' , 'R' , 'T' , 'Y' , 'U' , 'I' , 'O' , 'P' , '{' , '}' , '\n', 0x00, 'A' , 'S',  // 0x10
        'D' , 'F' , 'G' , 'H' , 'J' , 'K' , 'L' , ':' , '\"', '`' , 0x00, '|' , 'Z' , 'X' , 'C' , 'V',  // 0x20
        'B' , 'N' , 'M' , '<' , '>' , '?' , 0x00, '*' , 0x00, ' ' , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x30
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '7' , '8' , '9' , '-' , '4' , '5' , '6' , '+' , '1',  // 0x40
        '2' , '3' , '0' , '.' , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x50
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x60
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x70
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\n', 0x00, 0x00, 0x00, // 0x90
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xA0
        0x00, 0x00, 0x00, 0x00, 0x00, '/' , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xC0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xD0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xE0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 0xF0
    };

    /************************************************************************************************************/

    /// @brief Process a scan code for a shift key.
    int ProcessShift(int scanCode, bool isBreak)
    {
        ShiftFlags flag;

        switch (scanCode)
        {
        case 0x2A: case 0x36:
            flag = ShiftFlags::Shift;
            break;

        case 0x1D:
            flag = ShiftFlags::Ctrl;
            break;

        case 0x38:
            flag = ShiftFlags::Alt;
            break;

        default:
            return -1;
        }

        if (isBreak)
            s_shift -= flag;
        else
            s_shift += flag;

        return -1;
    }

    /************************************************************************************************************/

    /// @brief Process a scan code for a lock key.
    int ProcessLock(int scanCode, bool isBreak)
    {
        if (isBreak)
            return -1;

        switch (scanCode)
        {
        case 0x3A:
            s_shift ^= ShiftFlags::Caps;
            break;

        case 0x45:
            s_shift ^= ShiftFlags::Num;
            break;

        case 0x46:
            s_shift ^= ShiftFlags::Scroll;
            break;
        }

        return -1;
    }

    /************************************************************************************************************/

    /// @brief Process a scan code for a regular key.
    int ProcessRegular(int scanCode)
    {
        if (has_any(s_shift, ShiftFlags::Alt & ShiftFlags::Ctrl))
        //if (((s_shift & ShiftFlags::Alt) != 0) || ((s_shift & ShiftFlags::Ctrl) != 0))
            return -1;

        bool shifted = has_flags(s_shift, ShiftFlags::Shift);

        const char *map = shifted ? s_shiftMap : s_normalMap;
        int rval = map[scanCode];

        if (has_flags(s_shift, ShiftFlags::Caps))
        {
            // Magic case conversion (only works for ASCII tho)
            if (rval >= 'A' && rval <= 'Z')
                rval += 0x20;
            else if (rval >= 'a' && rval <= 'z')
                rval -= 0x20;
        }

        return rval ? rval : -1;
    }

    /************************************************************************************************************/

    /// @brief Core dispatch for all received scan codes.
    int ProcessScanCode(int scanCode)
    {
        KeyAction action = s_actions[scanCode];
        bool isBreak = false;

recheck:
        switch (action)
        {
        case KeyAction::Regular:
            if (has_flags(s_shift, ShiftFlags::Extend))
            {
                // Ignoring extended keys for now.
                s_lastCode = 0;
                s_shift -= ShiftFlags::Extend;
                return -1;
            }

            if (isBreak)
                return -1;

            return ProcessRegular(scanCode);

        case KeyAction::Pad:
            //return ProcessPad(scanCode);
            return -1;

        case KeyAction::Control:
            //return processControl(scanCode);
            return -1;

        case KeyAction::Shift:
            return ProcessShift(scanCode, isBreak);

        case KeyAction::Lock:
            return ProcessLock(scanCode, isBreak);

        case KeyAction::Ignore:
            if ((scanCode & 0x80) != 0)
            {
                isBreak = true;
                scanCode &= ~0x80;

                // Recheck after break flag removal.
                action = s_actions[scanCode];
                goto recheck;
            }
            return -1;

        case KeyAction::Extend:
            // Received an extended keycode prefix.
            s_lastCode = scanCode;
            s_shift += ShiftFlags::Extend;
            return -1;
        }

        return -1;
    }
}

/********************************************************************************************************************/

void keyboard_init()
{
    InitActionsTable();
}

extern "C" int terminal_getchar()
{
    uint8_t val = inb(STATUS_PORT);

    if ((val & STATUS_READ) == 0)
        return -1; // No data

    return ProcessScanCode(inb(DATA_PORT));
}

/********************************************************************************************************************/
