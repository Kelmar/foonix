/********************************************************************************************************************/
/********************************************************************************************************************/

#ifndef _CTYPE_H
#define _CTYPE_H 1

/********************************************************************************************************************/

// Not a fully comprehensive list of functions here, but most of the common ones.

inline
int isblank(int c) { return (c == ' ') || (c == '\t'); }

inline
int isspace(int c) { return isblank(c) || (c == '\v') || (c == '\r') || (c == '\n'); }

inline
int iscntrl(int c) { return (c < 32); }

inline
int isdigit(int c) { return (c >= '0') && (c <= '9'); }

inline
int isxdigit(int c) { return isdigit(c) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')); }

inline
int islower(int c) { return (c >= 'a') && (c <= 'z'); }

inline
int isupper(int c) { return (c >= 'A') && (c <= 'Z'); }

inline
int isalpha(int c) { return islower(c) || isupper(c); }

inline
int isalnum(int c) { return isdigit(c) || isalpha(c); }

inline
int tolower(int c) { return isupper(c) ? (c + 0x20) : c; }

inline
int toupper(int c) { return islower(c) ? (c - 0x20) : c; }

/********************************************************************************************************************/

#endif /* _CTYPE_H */

/********************************************************************************************************************/
