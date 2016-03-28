#ifndef _COS_COS_H_
#define _COS_COS_H_

#include <stddef.h>
#include <assert.h>

#define __COS__
#define __COS_NAME__        "COS"
#define __COS_VERSION__     "0.0.1"

#ifdef USE_CLANG_COMPLETER
#define container_of(ptr, type, mem)                        \
           ((type*)((char*)(ptr) - offsetof(type, mem)))
#else
#define container_of(ptr, type, mem)                        \
        ({ const typeof(((type*)0)->mem) *__mptr = (ptr);   \
           (type*)((char*)__mptr - offsetof(type, mem)); })  
#endif

#define MAIN_MEMORY_SIZE    (512 * 1024 * 1024)     // 512 MB
#define MAIN_MEMORY_SHIFT   (29)

#define PAGE_SIZE           (4096)
#define PAGE_SHIFT          (12)

#define DIRECT_MAPPED_START (0xC0000000u)
#define USER_SPACE_SIZE     (0x80000000u)
#define KERNEL_START        (USER_SPACE_SIZE)
#define KERNEL_SPACE_SIZE   (0x40000000u)
#define KERNEL_START_PAGE   (KERNEL_START >> PAGE_SHIFT)
#define KERNEL_PAGE_NUMBER  (KERNEL_SPACE_SIZE >> PAGE_SHIFT)

static_assert((1 << MAIN_MEMORY_SHIFT) == MAIN_MEMORY_SIZE, "memory size should match with memory shift");
static_assert((1 << PAGE_SHIFT) == PAGE_SIZE, "page size should match with page shift");

#endif 
