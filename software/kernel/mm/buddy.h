#ifndef _COS_MM_BUDDY_H_
#define _COS_MM_BUDDY_H_

#include "utils/linked-list.h"

#include "core/cos.h"

#define MM_BUDDY_SHIFT          (MAIN_MEMORY_SHIFT - PAGE_SHIFT)
#define MM_BUDDY_TREE_SIZE      ((MAIN_MEMORY_SIZE / PAGE_SIZE) * 2 * sizeof(BuddyNode))

typedef struct BuddyNode
{
    LinkedNode _link;
    size_t padding;
} BuddyNode;

typedef LinkedList BuddyHead;

typedef struct Buddy
{
    BuddyHead head[MM_BUDDY_SHIFT + 1];
    BuddyNode *tree;
    size_t free_nr;
} Buddy;

static_assert(sizeof(BuddyNode) == 16, "BuddyNode should be size of 16");

extern BuddyHead mm_buddy_heads[17];
extern BuddyNode *mm_buddy_tree;

void mm_buddy_init(Buddy *buddy, size_t reserved);
int mm_buddy_alloc(Buddy *buddy, size_t page_nr);
void mm_buddy_free(Buddy *buddy, int page);

#define mm_buddy_get_free_nr(buddy) ((buddy)->free_nr)

#endif
