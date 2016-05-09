#ifndef _C_LIB_STDLIB_H_
#define _C_LIB_STDLIB_H_

#include "../kernel/lib/sysapi.h"

static inline void *
malloc(size_t size)
{ return k_mmap_empty(size, -1); }

static inline void
free(void *ptr)
{ k_munmap(ptr); }

#endif
