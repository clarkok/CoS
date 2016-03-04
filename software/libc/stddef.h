#ifndef _C_LIB_STDDEF_
#define _C_LIB_STDDEF_

#include <stdint.h>

#define NULL    ((void*)0)
#define true    (1)
#define false   (0)

typedef uint32_t size_t;
typedef uint8_t bool;

#define offsetof(st, mem)   ((size_t)(&((st *)0)->mem))

#endif // _C_LIB_STDDEF_
