#include "ctype.h"

int
isalnum(int ch) __attribute__((always_inline))
{ return isalpha(ch) || isdigit(ch); }

int
isalpha(int ch) __attribute__((always_inline))
{ return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'); }

int
isblank(int ch) __attribute__((always_inline))
{ return (ch == '\t') || (ch == ' '); }

int
iscntrl(int ch) __attribute__((always_inline))
{ return (ch <= 0x1F) || (ch == 0x7F); }

int
isdigit(int ch) __attribute__((always_inline))
{ return (ch >= '0' && ch <= '9'); }

int
isgraph(int ch) __attribute__((always_inline))
{ return (ch >= 0x21 && ch <= 0x7E); }

int
islower(int ch) __attribute__((always_inline))
{ return (ch >= 'a' && ch <= 'z'); }

int
isprint(int ch) __attribute__((always_inline))
{ return (ch >= 0x20 && ch <= 0x7E); }

int
ispunct(int ch) __attribute__((always_inline))
{
    return (ch >= 0x21 && ch <= 0x2F) ||
           (ch >= 0x3A && ch <= 0x40) ||
           (ch >= 0x5B && ch <= 0x60) ||
           (ch >= 0x7B && ch <= 0x7E);
}

int
isspace(int ch) __attribute__((always_inline))
{ return (ch >= 0x09 && ch <= 0x0D) || (ch == ' '); }

int
isupper(int ch) __attribute__((always_inline))
{ return (ch >= 'A' && ch <= 'Z'); }

int
isxdigit(int ch) __attribute__((always_inline))
{ return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'); }
