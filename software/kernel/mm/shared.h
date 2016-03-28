#ifndef _COS_MM_SHARED_H_
#define _COS_MM_SHARED_H_

#include "utils/sb-tree.h"

#include "mm.h"

typedef struct SharedPages
{
    SBNode _node;

    size_t p_page_start;
    size_t page_count;
    size_t ref_count;
    int copy_on_write;
} SharedPages;

void mm_shared_init();

SharedPages *mm_shared_add_ref(size_t p_page_start, size_t page_count, int cow);
int mm_shared_rm_ref(SharedPages *shared_pages);

SharedPages *mm_shared_lookup(size_t p_page);

#endif
