#ifndef _COS_MM_H_
#define _COS_MM_H_

#include <stddef.h>
#include <assert.h>

#include "core/cos.h"
#include "utils/sb-tree.h"

#include "linked-buddy.h"

typedef struct PageDir
{
    unsigned valid      : 1;
    unsigned padding    : 11;
    unsigned page_ent   : 20;
} PageDir;

typedef struct PageEnt
{
    unsigned valid      : 1;
    unsigned we         : 1;
    unsigned padding    : 10;
    unsigned page       : 20;
} PageEnt;

#define PAGE_PER_DIR    (PAGE_SIZE / sizeof(PageDir))
#define MM_INVALID_PAGE (-1)

typedef struct MMFreeGroup
{
    SBNode _node;

    size_t v_page_start;
    size_t v_page_count;
} MMFreeGroup;

typedef struct MemoryManagement
{
    PageDir *page_table;
    SBTree space_map;
    LinkedBuddy *virtual_mem;
} MemoryManagement;

static_assert(sizeof(PageDir) == 4, "PageDir should be size of 4");
static_assert(sizeof(PageEnt) == 4, "PageEnt should be size of 4");

void mm_init();

void *malloc(size_t size);
void free(void *ptr);

void mm_init_proc(MemoryManagement *mm);
void mm_set_page_table(MemoryManagement *mm);

#endif
