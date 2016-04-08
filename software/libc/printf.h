#ifndef _C_LIB_PRINTF_H_
#define _C_LIB_PRINTF_H_

#include <stdarg.h>

int sprintf(char *buffer, const char *format, ...);
int vsprintf(char *buffer, const char *format, va_list vlist);

#endif
