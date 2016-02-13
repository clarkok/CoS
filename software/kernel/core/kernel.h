#ifndef _COS_KERNEL_H_
#define _COS_KERNEL_H_

#define __KERNEL__

#include <stdint.h>

#define __DEBUG__
#define __TEST__

#define KERNEL_BASE 0x80100000u     // 2GB + 1MB
#define MAPPED_BASE 0x80000000u     // 2GB
#define DIRECT_BASE 0xC0000000u     // 3GB

extern uint32_t kernel_size;

#define __ALWAYS_INLINE__   __attribute__((always_inline))
#define __WEAK__            __attribute__((weak))

#endif // _COS_KERNEL_H_
