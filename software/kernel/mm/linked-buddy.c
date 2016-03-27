#include <assert.h>

#include "utils/bits.h"

#include "mm.h"
#include "linked-buddy.h"

static inline size_t
_mm_linked_buddy_get_level_nr(size_t page_nr)
{ return bits_msb_idx_32(page_nr) + 1; }

static inline LinkedBuddyNode *
_mm_linked_buddy_node_new(int start)
{
    LinkedBuddyNode *ret = (LinkedBuddyNode*)malloc(sizeof(LinkedBuddyNode));
    if (!ret) return NULL;
    ret->start = start;
    return ret;
}

static inline LinkedBuddyAllocated *
_mm_linked_buddy_allocated_new(int start, int level)
{
    LinkedBuddyAllocated *ret = (LinkedBuddyAllocated*)malloc(sizeof(LinkedBuddyAllocated));
    if (!ret) return NULL;
    ret->start = start;
    ret->level = level;
    return ret;
}

static inline void
_mm_linked_buddy_allocated_insert(SBTree *tree, LinkedBuddyAllocated *node)
{
    SBNode **ptr = &sb_root(tree),
           *parent = NULL;
    while (*ptr) {
        parent = *ptr;
        if (sb_get(*ptr, LinkedBuddyAllocated, _node)->start > node->start) {
            ptr = &(*ptr)->left;
        }
        else {
            ptr = &(*ptr)->right;
        }
    }
    sb_link(&node->_node, parent, ptr, tree);
}

static inline LinkedBuddyAllocated *
_mm_linked_buddy_allocated_find(SBTree *tree, int start)
{
    SBNode *node = sb_root(tree);

    while (node) {
        LinkedBuddyAllocated *allocated = sb_get(node, LinkedBuddyAllocated, _node);
        if (allocated->start == start) return allocated;
        if (allocated->start > start) {
            node = node->left;
        }
        else {
            node = node->right;
        }
    }
    return NULL;
}

static inline int
_mm_linked_buddy_get_level(size_t page_nr, int level_nr)
{
    int msb = bits_msb_idx_32(page_nr);
    return level_nr - 1 - (((1 << msb) == page_nr) ? msb : msb + 1);
}

static inline int
_mm_linked_buddy_page_per_level(int level, int level_nr)
{ return (1 << (level_nr - level - 1)); }

static inline void
_mm_linked_buddy_insert_order(LinkedList *list, LinkedBuddyNode *node)
{
    LinkedNode *ptr = list_head(list);
    while (ptr && list_get(ptr, LinkedBuddyNode, _link)->start < node->start) {
        ptr = list_next(ptr);
    }

    if (!ptr) {
        list_append(list, &node->_link);
    }
    else {
        list_before(ptr, &node->_link);
    }
}

LinkedBuddy *
mm_linked_buddy_new(size_t page_nr)
{
    int level_nr = _mm_linked_buddy_get_level_nr(page_nr);
    LinkedBuddy *lbuddy = (LinkedBuddy*)malloc(sizeof(LinkedBuddy) + sizeof(LinkedList) * level_nr);

    if (!lbuddy) return NULL;

    lbuddy->level_nr = level_nr;
    lbuddy->free_nr = _mm_linked_buddy_page_per_level(0, level_nr);
    sb_init(&lbuddy->allocated);

    for (int i = 0; i < level_nr; ++i) {
        list_init(lbuddy->head + i);
    }

    LinkedBuddyNode *node = _mm_linked_buddy_node_new(0);
    list_append(lbuddy->head, &node->_link);

    return lbuddy;
}

void
mm_linked_buddy_destroy(LinkedBuddy *lbuddy)
{
    if (!lbuddy) return;

    for (int i = 0; i < lbuddy->level_nr; ++i) {
        while (list_size(lbuddy->head + i)) {
            LinkedNode *node = list_unlink(list_head(lbuddy->head + i));
            free(list_get(node, LinkedBuddyNode, _link));
        }
    }
    free(lbuddy);
}

int
mm_linked_buddy_alloc(LinkedBuddy *lbuddy, size_t page_nr)
{
    int level = _mm_linked_buddy_get_level(page_nr, lbuddy->level_nr);
    if (level < 0)                  { return MM_INVALID_PAGE; }
    if (page_nr > lbuddy->free_nr)  { return MM_INVALID_PAGE; }

    if (!list_size(lbuddy->head + level)) {
        int tmp_level = level - 1;
        while (tmp_level >= 0 && !list_size(lbuddy->head + tmp_level)) {
            --tmp_level;
        }
        if (tmp_level < 0) { return MM_INVALID_PAGE; }

        LinkedBuddyNode *node = list_get(
                list_unlink(list_head(lbuddy->head + tmp_level)),
                LinkedBuddyNode,
                _link
            );
        do {
            ++tmp_level;
            LinkedBuddyNode *new_node = _mm_linked_buddy_node_new(
                    node->start + _mm_linked_buddy_page_per_level(tmp_level, lbuddy->level_nr)
                );
            assert(!list_size(lbuddy->head + tmp_level));
            list_append(lbuddy->head + tmp_level, &new_node->_link);    // list should be empty now
        } while (tmp_level < level);
        list_prepend(lbuddy->head + level, &node->_link);
    }

    LinkedBuddyNode *node = list_get(
            list_unlink(list_head(lbuddy->head + level)),
            LinkedBuddyNode,
            _link
        );
    LinkedBuddyAllocated *allocated = _mm_linked_buddy_allocated_new(
            node->start,
            level
        );
    _mm_linked_buddy_allocated_insert(&lbuddy->allocated, allocated);

    int ret = node->start;
    free(node);

    lbuddy->free_nr -= _mm_linked_buddy_page_per_level(level, lbuddy->level_nr);
    return ret;
}

void
mm_linked_buddy_free(LinkedBuddy *lbuddy, int page)
{
    LinkedBuddyAllocated *allocated = _mm_linked_buddy_allocated_find(&lbuddy->allocated, page);
    assert(allocated);

    int level = allocated->level;
    sb_unlink(&allocated->_node);
    free(allocated);

    LinkedBuddyNode *node = _mm_linked_buddy_node_new(page);
    _mm_linked_buddy_insert_order(lbuddy->head + level, node);

    lbuddy->free_nr += _mm_linked_buddy_page_per_level(level, lbuddy->level_nr);

    while (1) {
        LinkedBuddyNode *buddy = NULL;
        int buddy_start = page ^ _mm_linked_buddy_page_per_level(level, lbuddy->level_nr);

        if (page & _mm_linked_buddy_page_per_level(level, lbuddy->level_nr)) {
            if (list_prev(&node->_link) && 
                list_get(list_prev(&node->_link), LinkedBuddyNode, _link)->start == buddy_start) {

                buddy = list_get(list_prev(&node->_link), LinkedBuddyNode, _link);
            }
        }
        else {
            if (list_next(&node->_link) && 
                list_get(list_next(&node->_link), LinkedBuddyNode, _link)->start == buddy_start) {

                buddy = list_get(list_next(&node->_link), LinkedBuddyNode, _link);
            }
        }

        if (!buddy) break;

        list_unlink(&node->_link);
        list_unlink(&buddy->_link);

        if (buddy->start < node->start) {
            free(node);
            node = buddy;
            page = buddy->start;
        }
        else {
            free(buddy);
        }

        _mm_linked_buddy_insert_order(lbuddy->head + --level, node);
    }
}
