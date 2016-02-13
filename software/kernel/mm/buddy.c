#include "utils/utils.h"

#include "buddy.h"
#include "mm_intl.h"

#ifdef __DEBUG__
#include "core/debug.h"
#endif // __DEBUG__

typedef struct BuddyNode 
{
    uint32_t prev;
    uint32_t next;
} BuddyNode;

static_assert((sizeof(BuddyNode) == 8), buddy_node_size_should_be_8_byte);

#define BUDDY_TREE_SIZE     ((MM_TOTAL_MEMORY / MM_PAGE_SIZE) * sizeof(BuddyNode) * 2)
#define BUDDY_TOTAL_SHIFT   (MM_TOTAL_MEMORY_SHIFT - MM_PAGE_SHIFT)

static BuddyNode *_mm_buddy_tree;
static uint32_t _mm_buddy_start[BUDDY_TOTAL_SHIFT + 1];

#define l_child_idx(idx)    ((idx) << 1)
#define r_child_idx(idx)    (((idx) << 1) + 1)
#define parent_idx(idx)     ((idx) >> 1)
#define idx_by_level(l)     (1 << (l))
#define size_by_level(l)    (1 << (BUDDY_TOTAL_SHIFT - l))

#define frame_by_idx(l, i)  (((i) - idx_by_level(l)) << (BUDDY_TOTAL_SHIFT - l))
#define node_is_valid(n)    (_mm_buddy_tree[n].next || _mm_buddy_tree[n].prev)

page_t _mm_buddy_get_level(page_t page_nr);
void _mm_buddy_prepend(page_t, page_t) __ALWAYS_INLINE__;
void _mm_buddy_remove(page_t, page_t) __ALWAYS_INLINE__;

#ifdef __DEBUG__
void _mm_buddy_output_debug();
#ifdef __TEST__
void _mm_buddy_test();
#endif // __TEST__
#endif // __DEBUG__

void
mm_buddy_init()
{
    _mm_map_to_physical(
            _kmm,
            (_kmm->heap_point + MAPPED_BASE + MM_PAGE_SIZE - 1) >> MM_PAGE_SHIFT,
            BUDDY_TREE_SIZE >> MM_PAGE_SHIFT,
            (_kmm->heap_point + MM_PAGE_SIZE - 1) >> MM_PAGE_SHIFT,
            true
        );
    _mm_buddy_tree = (BuddyNode*)(MAPPED_BASE + _kmm->heap_point);
    _kmm->heap_point += BUDDY_TREE_SIZE;

    for (int i = 0; i <= BUDDY_TOTAL_SHIFT; ++i) _mm_buddy_start[i] = MM_INVALID_PAGE;
    _mm_buddy_start[0] = 1;
    _mm_buddy_tree[1].prev = _mm_buddy_tree[1].next = MM_INVALID_PAGE;

#ifdef __DEBUG__
    _mm_buddy_output_debug();
#endif // __DEBUG__

    // reserved pages for kernel in buddy
    page_t k_page_nr = (_kmm->heap_point + MM_PAGE_SIZE - 1) >> MM_PAGE_SHIFT;
    page_t level = _mm_buddy_get_level(k_page_nr);

    page_t upper_level = 0;
    page_t node = 1;
    _mm_buddy_remove(upper_level, node);
    do {
        ++upper_level;
        _mm_buddy_prepend(upper_level, r_child_idx(node));
        node = l_child_idx(node);
    } while (upper_level != level);

#ifdef __DEBUG__
    _mm_buddy_output_debug();
#ifdef __TEST__
    _mm_buddy_test();
#endif // __TEST__
#endif // __DEBUG__
}

page_t
mm_buddy_alloc(page_t page_nr)
{
    page_t level = _mm_buddy_get_level(page_nr);
    page_t node = _mm_buddy_start[level];

    if (node == MM_INVALID_PAGE) {
        page_t upper_level = level - 1;
        while (upper_level && _mm_buddy_start[upper_level] == MM_INVALID_PAGE) {
            --upper_level;
        }

        if (!upper_level && _mm_buddy_start[upper_level] == MM_INVALID_PAGE) {
            // not found
            return MM_INVALID_PAGE;
        }

        node = _mm_buddy_start[upper_level];
        _mm_buddy_remove(upper_level, node);

        do {
            ++upper_level;
            _mm_buddy_prepend(upper_level, r_child_idx(node));
            node = l_child_idx(node);
        } while (upper_level != level);
    }
    else {
        _mm_buddy_remove(level, node);
    }

#ifdef __DEBUG__
    dbg_uart_str("\nBUDDY alloc ");
    dbg_uart_hex(page_nr);
    _mm_buddy_output_debug();
#endif // __DEBUG__

    return frame_by_idx(level, node);
}

void
mm_buddy_free(page_t start, page_t page_nr)
{
    page_t level = _mm_buddy_get_level(page_nr);
    page_t node = (start >> (BUDDY_TOTAL_SHIFT - level)) + idx_by_level(level);
    page_t buddy = node ^ 1;

    while (node_is_valid(buddy)) {
        _mm_buddy_remove(level, buddy);
        --level;
        node = parent_idx(node);
        buddy = node ^ 1;
    }
    _mm_buddy_prepend(level, node);

#ifdef __DEBUG__
    dbg_uart_str("\nBUDDY free ");
    dbg_uart_hex(start);
    dbg_uart_str(" ");
    dbg_uart_hex(page_nr);
    _mm_buddy_output_debug();
#endif // __DEBUG__
}

void
_mm_buddy_prepend(page_t level, page_t node) __ALWAYS_INLINE__
{
    _mm_buddy_tree[node].next = _mm_buddy_start[level];
    _mm_buddy_tree[node].prev = MM_INVALID_PAGE;
    if (_mm_buddy_start[level] != MM_INVALID_PAGE)
        _mm_buddy_tree[_mm_buddy_start[level]].prev = node;
    _mm_buddy_start[level] = node;
}

void
_mm_buddy_remove(page_t level, page_t node) __ALWAYS_INLINE__
{
    if (_mm_buddy_tree[node].prev == MM_INVALID_PAGE) {
        _mm_buddy_start[level] = _mm_buddy_tree[node].next;
    }
    else {
        _mm_buddy_tree[_mm_buddy_tree[node].prev].next = _mm_buddy_tree[node].next;
    }

    if (_mm_buddy_tree[node].next != MM_INVALID_PAGE) {
        _mm_buddy_tree[_mm_buddy_tree[node].next].prev = _mm_buddy_tree[node].prev;
    }
    _mm_buddy_tree[node].prev = _mm_buddy_tree[node].next = 0;
}

page_t
_mm_buddy_get_level(page_t page_nr) __ALWAYS_INLINE__
{
    int msb = get_msb_idx(page_nr);
    if ((1 << msb) == page_nr) { return BUDDY_TOTAL_SHIFT - msb; }
    else { return BUDDY_TOTAL_SHIFT - (msb + 1); }
}

#ifdef __DEBUG__
void
_mm_buddy_output_debug()
{
    dbg_uart_str("\nReporting Buddy:\n");
    for (page_t level = 0; level <= BUDDY_TOTAL_SHIFT; level++) {
        dbg_uart_str("  Level ");
        dbg_uart_hex(level);
        dbg_uart_str("(");
        dbg_uart_hex(_mm_buddy_start[level]);
        dbg_uart_str(") : ");
        if (_mm_buddy_start[level] == MM_INVALID_PAGE) {
            dbg_uart_str("No nodes valid\n");
        }
        else {
            page_t node = _mm_buddy_start[level];
            while (node != MM_INVALID_PAGE) {
                dbg_uart_hex(node);
                dbg_uart_str("(");
                dbg_uart_hex(_mm_buddy_tree[node].prev);
                dbg_uart_str(",");
                dbg_uart_hex(_mm_buddy_tree[node].next);
                dbg_uart_str(") ");
                node = _mm_buddy_tree[node].next;
            }
            dbg_uart_str("\n");
        }
    }
    dbg_uart_str("\n");
}

#ifdef __TEST__
void
_mm_buddy_test()
{
    page_t p10 = mm_buddy_alloc(10);
    page_t p1 = mm_buddy_alloc(1);

    mm_buddy_free(p1, 1);

    page_t p2 = mm_buddy_alloc(2);

    mm_buddy_free(p10, 10);
    mm_buddy_free(p2, 2);

    page_t p10_0 = mm_buddy_alloc(10);
    page_t p10_1 = mm_buddy_alloc(10);
    page_t p10_2 = mm_buddy_alloc(10);
    page_t p10_3 = mm_buddy_alloc(10);
    page_t p10_4 = mm_buddy_alloc(10);

    mm_buddy_free(p10_0, 10);
    mm_buddy_free(p10_3, 10);
    mm_buddy_free(p10_1, 10);
    mm_buddy_free(p10_2, 10);
    mm_buddy_free(p10_4, 10);
}
#endif // __TEST__
#endif // __DEBUG__
