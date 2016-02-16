#ifndef _COS_MM_H_
#define _COS_MM_H_

#include <debug.h>
#include <stddef.h>

#include "utils/bitmap.h"

#define MM_TOTAL_MEMORY         (512*1024*1024)
#define MM_TOTAL_MEMORY_SHIFT   (29)

#define MM_PAGE_SHIFT       (12)
#define MM_PAGE_SIZE        (1 << MM_PAGE_SHIFT)
#define MM_USER_SPACE_SIZE  (2*1024*1024*1024u)

#define MM_INVALID_PAGE     (0xFFFFFFFFu)

#define mm_get_direct_mapped_by_frame(frame)   \
    (DIRECT_BASE + ((frame) << 12))

static_assert((MM_PAGE_SIZE == 4096), page_size_shoule_be_4096);

typedef uint32_t addr_t;
typedef uint32_t page_t;

typedef struct MMPageEnt
{
    unsigned int valid  : 1;
    unsigned int we     : 1;
    unsigned int owner  : 10;
    unsigned int frame  : 20;
} MMPageEnt;

static_assert((sizeof(MMPageEnt) == 4), mm_page_entry_should_be_4_byte);

typedef struct MMPageDir
{
    unsigned int valid  : 1;
    unsigned int pad    : 1;
    unsigned int owner  : 10;
    unsigned int frame  : 20;
} MMPageDir;

static_assert((sizeof(MMPageDir) == 4), mm_page_dir_should_be_4_byte);

typedef struct BuddyAllocated
{
    unsigned int start  : 24;
    unsigned int shift  : 8;
} BuddyAllocated;

static_assert((sizeof(BuddyAllocated) == 4), buddy_allocated_should_by_4_byte);

// NOTE: this should align to MM_PAGE_SIZE
typedef struct MemoryManagement
{
    bitmap_t dir_bitmap[(MM_PAGE_SIZE - 2 * sizeof(size_t)) / sizeof(uint32_t)];
    size_t ent_allocated;
    addr_t heap_point;

    BuddyAllocated buddy_allocated[(MM_PAGE_SIZE / sizeof(BuddyAllocated)) - 1];
    size_t buddy_allocated_size;

    MMPageDir dir[MM_PAGE_SIZE / sizeof(MMPageDir)];

    MMPageEnt ent[0];
} MemoryManagement;

void mm_init();
addr_t mm_shift_heap_point(MemoryManagement*, addr_t);

#endif // _COS_MM_H_
