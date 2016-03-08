#ifndef _COS_SLAB_H_
#define _COS_SLAB_H_

#include <stddef.h>
#include <assert.h>

#include "utils/linked-list.h"

typedef void *(*SlabPageAlloc)();
typedef void (*SlabPageFree)(void *);

#define SLAB_SHIFT  (PAGE_SHIFT - 2)       // Minimum allocation unit is 4 Bytes

typedef LinkedNode SlabSlice;

typedef struct SlabPage
{
    LinkedNode _link;
    int level;
    LinkedList free_list;
    SlabSlice content[0];
} SlabPage;

typedef struct Slab
{
    LinkedList pages[SLAB_SHIFT];
    SlabPageAlloc alloc_page;
    SlabPageFree free_page;
} Slab;

void mm_slab_init(Slab *slab, SlabPageAlloc alloc_page, SlabPageFree free_page);
void *mm_slab_alloc(Slab *slab, size_t size);
void mm_slab_free(Slab *slab, void *ptr);

#endif
