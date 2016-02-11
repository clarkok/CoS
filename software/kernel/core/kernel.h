#ifndef _COS_KERNEL_H_
#define _COS_KERNEL_H_

#define __KERNEL__

#include <stdint.h>

#define KERNEL_BASE 0x8010000u  // 2GB + 1MB

extern uint32_t kernel_size;

#endif // _COS_KERNEL_H_
