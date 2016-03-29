#include <string.h>

#include "core/cos.h"

#include "utils/linked-list.h"
#include "proc/proc.h"

#include "mm.h"
#include "buddy.h"
#include "linked-buddy.h"
#include "slab.h"
#include "shared.h"

#define _MM_BUDDY_TREE_START    (BuddyNode *)(0x80400000)   // 4MB
#define _MM_BUDDY_RESERVED      (0x00800000 / PAGE_SIZE)

#define _MM_KERNEL_PTB          (PageDir*)(DIRECT_MAPPED_START + (0x00200000))

#define mm_get_dir_from_page(page)  ((page) >> 10)
#define mm_get_ent_from_page(page)  ((page) & ((1 << 10) - 1))

#define mm_get_direct_page_ptr(page)    (void*)((DIRECT_MAPPED_START + (page << PAGE_SHIFT)))
#define mm_get_direct_page_id(ptr)      (((size_t)ptr - DIRECT_MAPPED_START) >> PAGE_SHIFT)

#define mm_get_page_ptr(v_page)         (void*)((v_page) << PAGE_SHIFT)
#define mm_get_page_id(ptr)             ((size_t)ptr >> PAGE_SHIFT)

typedef LinkedNode MMPageForSlab;

typedef struct MMPageGroup
{
    SBNode _node;

    size_t v_page_start;    // start page number in virtual memory
    size_t p_page_start;    // start page number in physical memory
    size_t page_count;      // number of pages
    MMapType type;

    union {
        SharedPages *_shared_page;
    } _info;
} MMPageGroup;

#define i_shared_page   _info._shared_page

#define MIN_PAGES_FOR_SLAB  8
#define MAX_PAGES_FOR_SLAB  16
static LinkedList pages_for_slab;

static Buddy buddy;
static Slab kernel_slab;
static MemoryManagement kernel_mm;

static MMPageGroup *
_mm_page_group_insert(SBTree *tree, MMPageGroup *node)
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
_mm_page_group_find(SBTree *tree, size_t v_page)
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

static MMPageGroup *
_mm_page_group_lookup(SBTree *tree, size_t v_page)
{
    SBNode *ptr = sb_root(tree);

    while (ptr) {
        MMPageGroup *node = sb_get(ptr, MMPageGroup, _node);
        if (node->v_page_start <= v_page &&
            node->v_page_start + node->page_count > v_page) return node;
        if (node->v_page_start > v_page) {
            if (ptr->left) {
                ptr = ptr->left;
            }
            else if (!(ptr = sb_prev(ptr))) {
                return NULL;
            }
        }
        else {
            if (ptr->right) {
                ptr = ptr->right;
            }
            else if (!(ptr = sb_next(ptr))) {
                return NULL;
            }
        }
    }

    return NULL;
}

static inline PageEnt *
_mm_get_page_ent_from_v_page(PageDir *page_table, unsigned int v_page)
{
    unsigned int dir = mm_get_dir_from_page(v_page);

    assert(page_table[dir].valid);

    unsigned int ent_base = page_table[dir].page_ent;
    return ((PageEnt*)mm_get_direct_page_ptr(ent_base)) + mm_get_ent_from_page(v_page);
}

static inline void
_mm_set_kernel_page_table(PageDir *page_table, unsigned int v_page, unsigned int p_page, size_t page_nr, int we)
{
    while (page_nr --) {
        PageEnt *ent = _mm_get_page_ent_from_v_page(page_table, v_page);
        ent->valid = 1;
        ent->we = we;
        ent->page = p_page;

        ++p_page; ++v_page;
    }
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
}

static inline void
_mm_set_page_table(PageDir *page_table, unsigned int v_page, unsigned int p_page, size_t page_nr, int we)
{
    assert(v_page < KERNEL_START_PAGE);

    while (page_nr --) {
        unsigned int dir = mm_get_dir_from_page(v_page);
        if (!page_table[dir].valid) {
            int ent_page = mm_buddy_alloc(&buddy, 1);
            assert(ent_page != MM_INVALID_PAGE);

            PageEnt *ent = mm_get_direct_page_ptr(ent_page);
            memset(ent, 0, PAGE_SIZE);

            page_table[dir].page_ent = ent_page;
            page_table[dir].valid = 1;
        }

        PageEnt *ent = _mm_get_page_ent_from_v_page(page_table, v_page);
        ent->valid = 1;
        ent->we = we;
        ent->page = p_page;

        ++p_page; ++v_page;
    }
}

static inline void
_mm_reset_page_table(PageDir *page_table, unsigned int v_page, size_t page_nr)
{
    while (page_nr --) {
        PageEnt *ent = _mm_get_page_ent_from_v_page(page_table, v_page);
        ent->valid = 0;
        ent->we = 0;
        ent->page = 0;

        ++v_page;
    }
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

    // initial buddy
    buddy.tree = _MM_BUDDY_TREE_START;
    mm_buddy_init(&buddy, _MM_BUDDY_RESERVED);

    // initial kernel page table
    kernel_mm.page_table = _MM_KERNEL_PTB;

    // map kernel page_dir to first 1MB
    for (size_t i = 0; i < (KERNEL_SPACE_SIZE / PAGE_PER_DIR / PAGE_SIZE); ++i) {
        kernel_mm.page_table[i + (KERNEL_START / PAGE_PER_DIR / PAGE_SIZE)].page_ent = i;
        kernel_mm.page_table[i + (KERNEL_START / PAGE_PER_DIR / PAGE_SIZE)].valid = 1;
    }
    mm_update_mmu();

    // initial pages for slab
    list_init(&pages_for_slab);
    for (int i = 0; i < MAX_PAGES_FOR_SLAB; ++i) {
        page_for_slab[i] = mm_buddy_alloc(&buddy, 1);
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
    mm_update_mmu();

    // initial slab
    mm_slab_init(&kernel_slab, _mm_page_alloc_for_slab, _mm_page_free_for_slab);

    // initial kernel page groups
    sb_init(&kernel_mm.page_groups);

    // register reserved pages in kernel_mm.page_groups
    MMPageGroup *pg = (MMPageGroup*)malloc(sizeof(MMPageGroup));
    pg->v_page_start = (KERNEL_START >> PAGE_SHIFT);
    pg->p_page_start = 0;
    pg->page_count = _MM_BUDDY_RESERVED;
    pg->type = MM_EMPTY;
    _mm_page_group_insert(&kernel_mm.page_groups, pg);

    // register all pre-allocated pages for slab in kernel_mm.page_groups
    for (int i = 0; i < MAX_PAGES_FOR_SLAB; ++i) {
        pg = (MMPageGroup*)malloc(sizeof(MMPageGroup));
        pg->v_page_start = (KERNEL_START >> PAGE_SHIFT) + page_for_slab[i];
        pg->p_page_start = page_for_slab[i];
        pg->page_count = 1;
        pg->type = MM_EMPTY;
        _mm_page_group_insert(&kernel_mm.page_groups, pg);
    }

    // set kernel_mm.virtual_mem to NULL
    kernel_mm.virtual_mem = NULL;

    // initial shared memory
    mm_shared_init();
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
        mm_update_mmu();
        MMPageGroup *pg = (MMPageGroup*)mm_slab_alloc(&kernel_slab, sizeof(MMPageGroup));
        pg->v_page_start = page + (KERNEL_START >> PAGE_SHIFT);
        pg->p_page_start = page;
        pg->page_count = page_count;
        pg->type = MM_EMPTY;
        _mm_page_group_insert(&kernel_mm.page_groups, pg);

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
            mm_update_mmu();

            MMPageGroup *pg = (MMPageGroup*)mm_slab_alloc(&kernel_slab, sizeof(MMPageGroup));
            pg->v_page_start = page + (KERNEL_START >> PAGE_SHIFT);
            pg->p_page_start = page;
            pg->page_count = 1;
            pg->type = MM_EMPTY;
            _mm_page_group_insert(&kernel_mm.page_groups, pg);

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
        MMPageGroup *pg = _mm_page_group_find(&kernel_mm.page_groups, page);
        _mm_reset_kernel_page_table(
                kernel_mm.page_table,
                pg->v_page_start,
                pg->page_count
            );
        mm_update_mmu();
        sb_unlink(&pg->_node);
        mm_slab_free(&kernel_slab, pg);
    }
    else {
        mm_slab_free(&kernel_slab, ptr);

        while (list_size(&pages_for_slab) > MAX_PAGES_FOR_SLAB) {
            MMPageForSlab *page_for_slab = list_unlink(list_tail(&pages_for_slab));
            int page = ((size_t)page_for_slab - KERNEL_START) >> PAGE_SHIFT;
            MMPageGroup *pg = _mm_page_group_find(&kernel_mm.page_groups, page);
            _mm_reset_kernel_page_table(
                    kernel_mm.page_table,
                    pg->v_page_start,
                    pg->page_count
                );
            sb_unlink(&pg->_node);
            mm_slab_free(&kernel_slab, pg);
        }
        mm_update_mmu();
    }
}

void
mm_init_proc(MemoryManagement *mm)
{
    mm->page_table = (PageDir*)mm_get_direct_page_ptr(mm_buddy_alloc(&buddy, 1));
    assert((size_t)mm->page_table >= DIRECT_MAPPED_START);
    memset(mm->page_table, 0, PAGE_SIZE);
    memcpy(
            mm->page_table + KERNEL_START_PAGE / PAGE_PER_DIR,
            kernel_mm.page_table + KERNEL_START_PAGE / PAGE_PER_DIR,
            sizeof(PageDir) * KERNEL_PAGE_NUMBER / PAGE_PER_DIR
        );

    sb_init(&mm->page_groups);
    mm->virtual_mem = mm_linked_buddy_new(USER_SPACE_SIZE);
}

void
mm_destroy_proc(MemoryManagement *mm)
{
    mm_linked_buddy_destroy(mm->virtual_mem);

    for (SBNode *node = sb_head(&mm->page_groups); node; ) {
        MMPageGroup *pg = sb_get(node, MMPageGroup, _node);

        switch (pg->type) {
            case MM_EMPTY:
                {
                    mm_buddy_free(&buddy, pg->p_page_start);
                    break;
                }
            case MM_COW:
                {
                    if (!mm_shared_rm_ref(pg->i_shared_page)) {
                        mm_buddy_free(&buddy, pg->p_page_start);
                    }
                    break;
                }
            default:
                assert(false);
        }

        SBNode *next = sb_next(node);
        free(sb_unlink(node));
        node = next;
    }

    for (size_t i = 0; i < KERNEL_START_PAGE; ++i) {
        if (mm->page_table[i].valid) {
            mm_buddy_free(&buddy, mm->page_table[i].page_ent);
        }
    }

    mm_buddy_free(&buddy, mm_get_direct_page_id(mm->page_table));
}

void
mm_duplicate(MemoryManagement *dst, MemoryManagement *src)
{
    mm_init_proc(dst);

    sb_for_each(&src->page_groups, node) {
        MMPageGroup *pg = sb_get(node, MMPageGroup, _node);

        switch (pg->type) {
            case MM_EMPTY:
                {
                    pg->type = MM_COW;
                    pg->i_shared_page = mm_shared_add_ref(pg->p_page_start, pg->page_count, 1);
                    // fall through
                }
            case MM_COW:
                {
                    MMPageGroup *new_pg = (MMPageGroup*)malloc(sizeof(MMPageGroup));
                    assert(new_pg);
                    new_pg->v_page_start = pg->v_page_start;
                    new_pg->p_page_start = pg->p_page_start;
                    new_pg->page_count = pg->page_count;
                    new_pg->type = pg->type;
                    new_pg->i_shared_page = mm_shared_add_ref(pg->p_page_start, pg->page_count, 1);

                    _mm_set_page_table(src->page_table, pg->v_page_start, pg->p_page_start, pg->page_count, 0);
                    _mm_set_page_table(dst->page_table, new_pg->v_page_start, new_pg->p_page_start, new_pg->page_count, 0);

                    break;
                }
            default:
                assert(false);
        }
    }

    dst->virtual_mem = mm_linked_buddy_dup(src->virtual_mem);
}

void
mm_set_page_table(MemoryManagement *mm)
{
    assert((size_t)mm->page_table >= (size_t)DIRECT_MAPPED_START);
    out_ptb((size_t)mm->page_table - (size_t)DIRECT_MAPPED_START);
}

void *
mm_do_mmap_empty(size_t size, size_t hint)
{
    // FIXME protect with lock

    MemoryManagement *mm = &current_process->mm;

    int page_nr = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
    int p_page = mm_buddy_alloc(&buddy, page_nr);
    if (p_page == MM_INVALID_PAGE) { return NULL; }

    int v_page;
    if ((int)hint == MM_INVALID_PAGE) {
        v_page = mm_linked_buddy_alloc(mm->virtual_mem, page_nr);
    }
    else {
        v_page = mm_linked_buddy_alloc_hint(mm->virtual_mem, page_nr, hint >> PAGE_SHIFT);
    }

    if (v_page == MM_INVALID_PAGE) {
        mm_buddy_free(&buddy, p_page);
        return NULL;
    }

    MMPageGroup *pg = (MMPageGroup*)malloc(sizeof(MMPageGroup));
    pg->v_page_start = v_page;
    pg->p_page_start = p_page;
    pg->page_count = page_nr;
    pg->type = MM_EMPTY;

    _mm_page_group_insert(&mm->page_groups, pg);

    _mm_set_page_table(mm->page_table, v_page, p_page, page_nr, 1);

    mm_update_mmu();
    return mm_get_page_ptr(v_page);
}

void
mm_do_munmap(void *ptr)
{
    // FIXME protect with lock
    MemoryManagement *mm = &current_process->mm;

    int v_page = mm_get_page_id(ptr);
    MMPageGroup *pg = _mm_page_group_find(&mm->page_groups, v_page);
    if (!pg) { return; }

    mm_linked_buddy_free(mm->virtual_mem, pg->v_page_start);

    switch (pg->type) {
        case MM_COW:
            {
                if (!mm_shared_rm_ref(pg->i_shared_page)) {
                    mm_buddy_free(&buddy, pg->p_page_start);
                }
                break;
            }
        case MM_EMPTY: break;
        default: assert(false);
    }

    _mm_reset_page_table(mm->page_table, pg->v_page_start, pg->page_count);

    sb_unlink(&pg->_node);
    free(pg);

    mm_update_mmu();
}

void
mm_pagefault_handler()
{
    size_t pagefault_addr = in_pfa();
    MemoryManagement *mm = &current_process->mm;

    if (pagefault_addr >= USER_SPACE_SIZE) {
        dbg_uart_str("Kernel Pagefault!\n");
        dbg_uart_str("PFA: ");
        dbg_uart_hex(pagefault_addr);
        kernel_panic();
    }

    MMPageGroup *pg = _mm_page_group_lookup(&mm->page_groups, pagefault_addr >> PAGE_SHIFT);

    if (!pg) {
        // TODO terminate current_process
        assert(false && "Should terminate current process");
    }

    switch (pg->type) {
        case MM_EMPTY:
            // TODO terminate current_process
            assert(false && "Should terminate current process");
            break;
        case MM_COW:
            if (pg->i_shared_page->ref_count == 1) {
                mm_shared_rm_ref(pg->i_shared_page);
                pg->type = MM_EMPTY;
                _mm_set_page_table(mm->page_table, pg->v_page_start, pg->p_page_start, pg->page_count, 1);
            }
            else {
                mm_shared_rm_ref(pg->i_shared_page);
                int new_pages = mm_buddy_alloc(&buddy, pg->page_count);
                assert(new_pages != MM_INVALID_PAGE);

                memcpy(
                        mm_get_direct_page_ptr(new_pages),
                        mm_get_direct_page_ptr(pg->p_page_start),
                        pg->page_count << PAGE_SHIFT
                    );

                _mm_reset_page_table(mm->page_table, pg->v_page_start, pg->page_count);
                pg->p_page_start = new_pages;
                _mm_set_page_table(mm->page_table, pg->v_page_start, pg->p_page_start, pg->page_count, 1);
            }
            mm_update_mmu();
            break;
        default:
            assert(false);
    }
}
