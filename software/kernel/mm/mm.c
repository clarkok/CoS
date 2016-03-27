#include "core/kernel.h"

#include "utils/linked-list.h"

#include "mm.h"
#include "buddy.h"
#include "slab.h"

#define _MM_BUDDY_TREE_START    (BuddyNode *)(0x80400000)   // 4MB
#define _MM_BUDDY_RESERVED      (0x00800000 / PAGE_SIZE)

#define _MM_KERNEL_PTB          (PageDir*)(DIRECT_MAPPED_START + (0x00200000))

#define mm_get_dir_from_page(page)  ((page) >> 10)
#define mm_get_ent_from_page(page)  ((page) & ((1 << 10) - 1))

#define mm_get_kernel_dir_page(dir) (dir)

typedef LinkedNode MMPageForSlab;

typedef struct MMPageGroup
{
    SBNode _node;

    size_t v_page_start;    // start page number in virtual memory
    size_t p_page_start;    // start page number in physical memory
    size_t page_count;      // number of pages
} MMPageGroup;

#define MIN_PAGES_FOR_SLAB  8
#define MAX_PAGES_FOR_SLAB  16
static LinkedList pages_for_slab;

static Buddy buddy;
static Slab kernel_slab;
static MemoryManagement kernel_mm;

static MMPageGroup *
_page_group_insert(SBTree *tree, MMPageGroup *node)
{
    SBNode **link = &sb_root(tree),
           *parent = NULL;

    while (*link) {
        parent = *link;
        if (sb_get(*link, MMPageGroup, _node)->v_page_start > node->v_page_start) {
            link = &(*link)->left;
        }
        else {
            link = &(*link)->right;
        }
    }

    sb_link(&node->_node, parent, link, tree);

    return node;
}

static MMPageGroup *
_page_group_find(SBTree *tree, size_t v_page)
{
    SBNode *ptr = sb_root(tree);

    while (ptr) {
        MMPageGroup *node = sb_get(ptr, MMPageGroup, _node);
        if (node->v_page_start == v_page)   return node;
        if (node->v_page_start > v_page)    ptr = ptr->left;
        else                                ptr = ptr->right;
    }

    return NULL;
}

static inline PageEnt *
_mm_get_page_ent_from_v_page(PageDir *page_table, unsigned int v_page)
{
    unsigned int dir = mm_get_dir_from_page(v_page);

    assert(page_table[dir].valid);

    unsigned int ent_base = page_table[dir].page_ent;
    return ((PageEnt*)(DIRECT_MAPPED_START + (ent_base << PAGE_SHIFT))) + mm_get_ent_from_page(v_page);
}

static inline void
_mm_set_kernel_page_table(PageDir *page_table, unsigned int v_page, unsigned int p_page, size_t page_nr, int we)
{
    while (page_nr --) {
        unsigned int dir = mm_get_dir_from_page(v_page);
        if (!page_table[dir].valid) {
            page_table[dir].page_ent = mm_get_kernel_dir_page(dir);
            page_table[dir].valid = 1;
        }

        PageEnt *ent = _mm_get_page_ent_from_v_page(page_table, v_page);
        ent->valid = 1;
        ent->we = we;
        ent->page = p_page;

        ++p_page; ++v_page;
    }
    out_ptb((size_t)page_table - DIRECT_MAPPED_START);
}

static inline void
_mm_reset_kernel_page_table(PageDir *page_table, unsigned int v_page, size_t page_nr)
{
    while (page_nr --) {
        PageEnt *ent = _mm_get_page_ent_from_v_page(page_table, v_page);
        ent->valid = 0;
        ent->we = 0;
        ent->page = 0;

        ++v_page;
    }

    out_ptb((size_t)page_table - DIRECT_MAPPED_START);
}

static inline void *
_mm_page_alloc_for_slab()
{ return list_unlink(list_head(&pages_for_slab)); }

static inline void
_mm_page_free_for_slab(void *ptr)
{ list_prepend(&pages_for_slab, (MMPageForSlab*)ptr); }

void
mm_init()
{
    int page_for_slab[MAX_PAGES_FOR_SLAB];

    buddy.tree = _MM_BUDDY_TREE_START;
    mm_buddy_init(&buddy, _MM_BUDDY_RESERVED);

    kernel_mm.page_table = _MM_KERNEL_PTB;

    list_init(&pages_for_slab);
    for (int i = 0; i < MAX_PAGES_FOR_SLAB; ++i) {
        page_for_slab[i] = mm_buddy_alloc(&buddy, 1);
        assert(page_nr != MM_INVALID_PAGE);
        assert(page_for_slab[i] == (i + _MM_BUDDY_RESERVED));

        _mm_set_kernel_page_table(
                kernel_mm.page_table,
                page_for_slab[i] + (KERNEL_START >> PAGE_SHIFT),
                page_for_slab[i],
                1,
                1
            );
        MMPageForSlab *page = (MMPageForSlab*)(KERNEL_START + (page_for_slab[i] << PAGE_SHIFT));
        list_append(&pages_for_slab, page);
    }

    mm_slab_init(&kernel_slab, _mm_page_alloc_for_slab, _mm_page_free_for_slab);

    sb_init(&kernel_mm.space_map);

    MMPageGroup *pg = (MMPageGroup*)malloc(sizeof(MMPageGroup));
    pg->v_page_start = (KERNEL_START >> PAGE_SHIFT);
    pg->p_page_start = 0;
    pg->page_count = _MM_BUDDY_RESERVED;
    _page_group_insert(&kernel_mm.space_map, pg);

    for (int i = 0; i < MAX_PAGES_FOR_SLAB; ++i) {
        pg = (MMPageGroup*)malloc(sizeof(MMPageGroup));
        pg->v_page_start = (KERNEL_START >> PAGE_SHIFT) + page_for_slab[i];
        pg->p_page_start = page_for_slab[i];
        pg->page_count = 1;
        _page_group_insert(&kernel_mm.space_map, pg);
    }

    kernel_mm.virtual_mem = NULL;
}

void *
malloc(size_t size)
{
    if (size > (PAGE_SIZE >> 1)) {
        int page_count = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
        int page = mm_buddy_alloc(&buddy, page_count);
        assert(page != MM_INVALID_PAGE);

        _mm_set_kernel_page_table(
                kernel_mm.page_table,
                page + (KERNEL_START >> PAGE_SHIFT),
                page,
                page_count,
                1
            );
        MMPageGroup *pg = (MMPageGroup*)mm_slab_alloc(&kernel_slab, sizeof(MMPageGroup));
        pg->v_page_start = page + (KERNEL_START >> PAGE_SHIFT);
        pg->p_page_start = page;
        pg->page_count = page_count;
        _page_group_insert(&kernel_mm.space_map, pg);

        return (void*)(KERNEL_START + (page << PAGE_SHIFT));
    }
    else {
        void *ret = mm_slab_alloc(&kernel_slab, size);

        while (list_size(&pages_for_slab) < MIN_PAGES_FOR_SLAB) {
            int page = mm_buddy_alloc(&buddy, 1);
            assert(page != MM_INVALID_PAGE);

            _mm_set_kernel_page_table(
                    kernel_mm.page_table,
                    page + (KERNEL_START >> PAGE_SHIFT),
                    page,
                    1,
                    1
                );

            MMPageGroup *pg = (MMPageGroup*)mm_slab_alloc(&kernel_slab, sizeof(MMPageGroup));
            pg->v_page_start = page + (KERNEL_START >> PAGE_SHIFT);
            pg->p_page_start = page;
            pg->page_count = 1;
            _page_group_insert(&kernel_mm.space_map, pg);

            MMPageForSlab *page_for_slab = (MMPageForSlab*)(KERNEL_START + (page << PAGE_SHIFT));
            list_append(&pages_for_slab, page_for_slab);
        }

        return ret;
    }
}

void
free(void *ptr)
{
    if (!((size_t)ptr & (PAGE_SIZE - 1))) {
        int page = ((size_t)ptr - KERNEL_START) >> PAGE_SHIFT;
        MMPageGroup *pg = _page_group_find(&kernel_mm.space_map, page);
        _mm_reset_kernel_page_table(
                kernel_mm.page_table,
                pg->v_page_start,
                pg->page_count
            );
        sb_unlink(&pg->_node);
        mm_slab_free(&kernel_slab, pg);
    }
    else {
        mm_slab_free(&kernel_slab, ptr);

        while (list_size(&pages_for_slab) > MAX_PAGES_FOR_SLAB) {
            MMPageForSlab *page_for_slab = list_unlink(list_tail(&pages_for_slab));
            int page = ((size_t)page_for_slab - KERNEL_START) >> PAGE_SHIFT;
            MMPageGroup *pg = _page_group_find(&kernel_mm.space_map, page);
            _mm_reset_kernel_page_table(
                    kernel_mm.page_table,
                    pg->v_page_start,
                    pg->page_count
                );
            sb_unlink(&pg->_node);
            mm_slab_free(&kernel_slab, pg);
        }
    }
}
