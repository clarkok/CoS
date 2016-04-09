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
    LinkedBuddyNode *ret = (LinkedBuddyNode*)kmalloc(sizeof(LinkedBuddyNode));
    if (!ret) return NULL;
    ret->start = start;
    return ret;
}

static inline LinkedBuddyAllocated *
_mm_linked_buddy_allocated_new(int start, int level)
{
    LinkedBuddyAllocated *ret = (LinkedBuddyAllocated*)kmalloc(sizeof(LinkedBuddyAllocated));
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
    LinkedBuddy *lbuddy = (LinkedBuddy*)kmalloc(sizeof(LinkedBuddy) + sizeof(LinkedList) * level_nr);

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

LinkedBuddy *
mm_linked_buddy_dup(LinkedBuddy *lbuddy)
{
    LinkedBuddy *ret = (LinkedBuddy*)kmalloc(sizeof(LinkedBuddy) + sizeof(LinkedList) * lbuddy->level_nr);
    if (!ret) return NULL;

    ret->level_nr = lbuddy->level_nr;
    ret->free_nr = lbuddy->free_nr;

    sb_for_each(&lbuddy->allocated, node) {
        LinkedBuddyAllocated *allocated = sb_get(node, LinkedBuddyAllocated, _node);
        LinkedBuddyAllocated *new_alloc = kmalloc(sizeof(LinkedBuddyAllocated));

        new_alloc->start = allocated->start;
        new_alloc->level = allocated->level;

        _mm_linked_buddy_allocated_insert(&ret->allocated, new_alloc);
    }

    for (int i = 0; i < lbuddy->level_nr; ++i) {
        list_init(ret->head + i);

        list_for_each(lbuddy->head + i, node) {
            LinkedBuddyNode *buddy_node = list_get(node, LinkedBuddyNode, _link),
                            *new_node = kmalloc(sizeof(LinkedBuddyNode));

            new_node->start = buddy_node->start;
            list_append(ret->head + i, &new_node->_link);
        }
    }

    return ret;
}

void
mm_linked_buddy_destroy(LinkedBuddy *lbuddy)
{
    if (!lbuddy) return;

    for (int i = 0; i < lbuddy->level_nr; ++i) {
        while (list_size(lbuddy->head + i)) {
            LinkedNode *node = list_unlink(list_head(lbuddy->head + i));
            kfree(list_get(node, LinkedBuddyNode, _link));
        }
    }
    kfree(lbuddy);
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
    kfree(node);

    lbuddy->free_nr -= _mm_linked_buddy_page_per_level(level, lbuddy->level_nr);
    return ret;
}

static inline LinkedBuddyNode *
_mm_linked_buddy_search_hint(LinkedBuddy *lbuddy, int level, int start)
{
    list_for_each(lbuddy->head + level, node) {
        if (list_get(node, LinkedBuddyNode, _link)->start == start) {
            return list_get(node, LinkedBuddyNode, _link);
        }
    }
    return NULL;
}

static inline int
_mm_linked_buddy_align_hint(LinkedBuddy *lbuddy, int level, int hint)
{ return hint & -_mm_linked_buddy_page_per_level(level, lbuddy->level_nr); }

int
mm_linked_buddy_alloc_hint(LinkedBuddy *lbuddy, size_t page_nr, int hint)
{
    int level = _mm_linked_buddy_get_level(page_nr, lbuddy->level_nr);
    if (level < 0)                      { return MM_INVALID_PAGE; }
    if (page_nr > lbuddy->free_nr)      { return MM_INVALID_PAGE; }

    hint = _mm_linked_buddy_align_hint(lbuddy, level, hint);
    LinkedBuddyNode *node = _mm_linked_buddy_search_hint(lbuddy, level, hint);
    if (!node) {
        int tmp_level = level - 1;
        int tmp_hint = hint;
        LinkedBuddyNode *tmp_node;

        while (tmp_level >= 0) {
            tmp_hint = _mm_linked_buddy_align_hint(lbuddy, tmp_level, tmp_hint);
            tmp_node = _mm_linked_buddy_search_hint(lbuddy, tmp_level, tmp_hint);

            if (tmp_node) break;
            --tmp_level;
        }

        if (tmp_level < 0) return MM_INVALID_PAGE;

        list_unlink(&tmp_node->_link);
        do {
            ++tmp_level;
            LinkedBuddyNode *new_node = _mm_linked_buddy_node_new(
                    tmp_node->start + _mm_linked_buddy_page_per_level(tmp_level, lbuddy->level_nr)
                );
            if (hint < new_node->start) {
                _mm_linked_buddy_insert_order(lbuddy->head + tmp_level, new_node);
            }
            else {
                _mm_linked_buddy_insert_order(lbuddy->head + tmp_level, tmp_node);
                tmp_node = new_node;
            }
        } while (tmp_level < level);
        node = tmp_node;
    }
    else {
        list_unlink(&node->_link);
    }

    LinkedBuddyAllocated *allocated = _mm_linked_buddy_allocated_new(
            node->start,
            level
        );
    _mm_linked_buddy_allocated_insert(&lbuddy->allocated, allocated);

    int ret = node->start;
    kfree(node);

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
    kfree(allocated);

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
            kfree(node);
            node = buddy;
            page = buddy->start;
        }
        else {
            kfree(buddy);
        }

        _mm_linked_buddy_insert_order(lbuddy->head + --level, node);
    }
}
