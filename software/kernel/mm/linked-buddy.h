#ifndef _COS_LINKED_BUDDY_H_
#define _COS_LINKED_BUDDY_H_

#include <stddef.h>
#include "utils/linked-list.h"
#include "utils/sb-tree.h"

typedef struct LinkedBuddyNode
{
    LinkedNode _link;
    int start;
} LinkedBuddyNode;

typedef struct LinkedBuddyAllocated
{
    SBNode _node;

    int start;
    int level;
} LinkedBuddyAllocated;

typedef struct LinkedBuddy
{
    int level_nr;
    size_t free_nr;
    SBTree allocated;
    LinkedList head[0];
} LinkedBuddy;

LinkedBuddy *mm_linked_buddy_new(size_t page_nr);
LinkedBuddy *mm_linked_buddy_dup(LinkedBuddy *lbuddy);

void mm_linked_buddy_destroy(LinkedBuddy *lbuddy);
int mm_linked_buddy_alloc(LinkedBuddy *lbuddy, size_t page_nr);
void mm_linked_buddy_free(LinkedBuddy *lbuddy, int page);

#endif
