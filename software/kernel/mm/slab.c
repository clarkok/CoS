#include "utils/bits.h"

#include "slab.h"

void
mm_slab_init(Slab *slab, SlabPageAlloc alloc_page, SlabPageFree free_page)
{
    for (int i = 0; i < SLAB_SHIFT; ++i) {
        list_init(slab->pages + i);
    }

    slab->alloc_page = alloc_page;
    slab->free_page = free_page;
}

static inline int
_mm_slab_get_level(size_t size)
{
    int msb = bits_msb_idx_32(size);
    return ((size == (1 << msb)) ? msb : (msb + 1)) - SLAB_UNIT_SHIFT;
}

static inline size_t
_mm_slab_slice_nr_in_page(int level)
{ return (PAGE_SIZE - sizeof(SlabPage)) >> (level + SLAB_UNIT_SHIFT); }

static inline SlabPage *
_mm_slab_new_page(Slab *slab, int level)
{
    SlabPage *page = (SlabPage *)slab->alloc_page();
    assert(page);
    list_init(&page->free_list);
    page->level = level;

    SlabSlice *slice = page->content,
              *limit = (SlabSlice*)((size_t)(page->content) + (_mm_slab_slice_nr_in_page(level) << (level + SLAB_UNIT_SHIFT)));

    while (slice < limit) {
        list_append(&page->free_list, slice);
        slice = (SlabSlice*)((size_t)slice + (1 << (level + SLAB_UNIT_SHIFT)));
    }

    return page;
}

void *
mm_slab_alloc(Slab *slab, size_t size)
{
    if (size < SLAB_UNIT) size = SLAB_UNIT;

    int level = _mm_slab_get_level(size);
    assert(level < SLAB_SHIFT);

    if (!list_size(slab->pages + level)) {
        SlabPage *page = _mm_slab_new_page(slab, level);
        list_prepend(slab->pages + level, &page->_link);
    }

    SlabPage *page = list_get(list_head(slab->pages + level), SlabPage, _link);
    SlabSlice *slice = list_unlink(list_head(&page->free_list));

    if (!list_size(&page->free_list)) {
        list_unlink(&page->_link);
    }

    return (void*)slice;
}

void
mm_slab_free(Slab *slab, void *ptr)
{
    SlabPage *page = (SlabPage*)((size_t)ptr & ~(PAGE_SIZE - 1));

    list_prepend(&page->free_list, (LinkedNode*)ptr);

    if (list_size(&page->free_list) == 1) {
        list_append(slab->pages + page->level, &page->_link);
    }
    else if (list_size(&page->free_list) == _mm_slab_slice_nr_in_page(page->level)) {
        if (list_size(slab->pages + page->level) > 1) {
            list_unlink(&page->_link);
            slab->free_page(page);
        }
    }
}
