#ifndef _C_LIB_STDARG_H_
#define _C_LIB_STDARG_H_

#include <stddef.h>

typedef char *va_list;

#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)

#endif
