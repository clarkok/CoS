#include <string.h>
#include <assert.h>

#include "utils/bits.h"
#include "utils/linked-list.h"

#include "mm.h"
#include "buddy.h"

void
mm_buddy_init(Buddy *buddy, size_t reserved)
{
    for (int i = 0; i < MM_BUDDY_SHIFT; ++i) {
        list_init(buddy->head + i);
    }
    memset(buddy->tree, 0, MM_BUDDY_TREE_SIZE);
    list_prepend(buddy->head, &buddy->tree[1]._link);
    buddy->free_nr = 1 << (MM_BUDDY_SHIFT - 1);

    if (reserved) {
        mm_buddy_alloc(buddy, reserved);
    }
}

static inline int
_mm_buddy_get_level(size_t page_nr)
{
    int msb = bits_msb_idx_32(page_nr);
    return MM_BUDDY_SHIFT - ((1 << msb) == page_nr ? msb : (msb + 1));
}

static inline int
_mm_buddy_get_page_from_index(int index)
{
    int msb = bits_msb_idx_32(index);
    return (index - (1 << msb)) << (MM_BUDDY_SHIFT - msb);
}

static inline int
_mm_buddy_get_index_from_page(int level, int page)
{ return (page >> (MM_BUDDY_SHIFT - level)) + (1 << level); }

int
mm_buddy_alloc(Buddy *buddy, size_t page_nr)
{
    int level = _mm_buddy_get_level(page_nr);

    if (!list_size(buddy->head + level)) {
        int t_level = level - 1;
        while (t_level && !list_size(buddy->head + t_level)) {
            --t_level;
        }

        if (!t_level && !list_size(buddy->head)) return MM_INVALID_PAGE;

        while (t_level != level) {
            LinkedNode *node = list_unlink(list_head(buddy->head + t_level));
            BuddyNode *b_node = list_get(node, BuddyNode, _link);
            int index = b_node - buddy->tree;
            list_append(buddy->head + t_level + 1, &buddy->tree[(index << 1)]._link);
            list_append(buddy->head + t_level + 1, &buddy->tree[(index << 1) + 1]._link);
            ++t_level;
        }
    }

    LinkedNode *node = list_unlink(list_head(buddy->head + level));
    BuddyNode *b_node = list_get(node, BuddyNode, _link);
    int index = b_node - buddy->tree;
    int page = _mm_buddy_get_page_from_index(index);

    buddy->tree[page + (1 << (MM_BUDDY_SHIFT - 1))].padding = level;
    buddy->free_nr -= (1 << (MM_BUDDY_SHIFT - level));

    return page;
}

void
mm_buddy_free(Buddy *buddy, int page)
{
    int level = buddy->tree[page + (1 << (MM_BUDDY_SHIFT - 1))].padding;
    assert(level);
    buddy->free_nr += (1 << (MM_BUDDY_SHIFT - level));

    int index = _mm_buddy_get_index_from_page(level, page);
    list_prepend(buddy->head + level, &buddy->tree[index]._link);
    buddy->tree[page + (1 << (MM_BUDDY_SHIFT - 1))].padding = 0;

    int b_index = index ^ 1;

    while (level && list_node_linked(&buddy->tree[b_index]._link)) {
        list_unlink(list_head(buddy->head + level));
        list_unlink(&buddy->tree[b_index]._link);

        index >>= 1;
        b_index = index ^ 1;

        list_prepend(buddy->head + --level, &buddy->tree[index]._link);
    }
}
