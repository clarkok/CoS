#ifndef _COS_MM_H_
#define _COS_MM_H_

#include <debug.h>
#include <stddef.h>

#define MM_PAGE_SIZE        (4096)
#define MM_USER_SPACE_SIZE  (2*1024*1024*1024u)

static_assert((MM_PAGE_SIZE == 4096), page_size_shoule_be_4096);

struct MMPageEnt
{
    unsigned int valid  : 1;
    unsigned int we     : 1;
    unsigned int owner  : 10;
    unsigned int id     : 20;
};

static_assert((sizeof(struct MMPageEnt) == 4), mm_page_entry_should_be_4_byte);

struct MMPageDir
{
    unsigned int valid  : 1;
    unsigned int pad    : 1;
    unsigned int owner  : 10;
    unsigned int id     : 20;
};

static_assert((sizeof(struct MMPageDir) == 4), mm_page_dir_should_be_4_byte);

// NOTE: this should align to MM_PAGE_SIZE
struct MemoryManagement
{
    uint32_t ent_mask[(MM_PAGE_SIZE - 2 * sizeof(size_t)) / sizeof(uint32_t)];
    size_t ent_allocated;
    size_t heap_point;
    struct MMPageDir dir[MM_PAGE_SIZE / sizeof(struct MMPageDir)];
    struct MMPageEnt ent[0];
};

void mm_init();

#endif // _COS_MM_H_
