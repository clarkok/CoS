#include "core/kernel.h"

#include "mm.h"

#define BUDDY_TREE_SIZE         (1*1024*1024)   // 1MB
#define KERNEL_PAGE_TABLE_SIZE  (1*1024*1024)   // 1MB

static uint32_t *mm_buddy_tree;

void
mm_init()
{
    mm_buddy_tree = (uint32_t*)(KERNEL_BASE + 
                        (kernel_size + BUDDY_TREE_SIZE) & -BUDDY_TREE_SIZE);
}
