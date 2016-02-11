#include "core/kernel.h"
#include "core/debug.h"

#include "utils/utils.h"

#include "mm.h"

#define BUDDY_TREE_SIZE         (1*1024*1024)   // 1MB
#define KERNEL_PAGE_TABLE_SIZE  (1*1024*1024)   // 1MB

#define ENT_PER_PAGE    (MM_PAGE_SIZE / sizeof(MMPageEnt))

static uint32_t *mm_buddy_tree;
MemoryManagement *kmm;

static void _mm_map_to_physical(MemoryManagement *, addr_t, size_t, addr_t, bool);
static void _mm_fill_page_ent(MMPageEnt *, size_t, addr_t, bool);
static addr_t _mm_get_page_dir(addr_t);
static addr_t _mm_get_page_ent(addr_t);
static addr_t _mm_get_page_no(addr_t);
static addr_t _mm_get_page_dir_from_page(page_t);

void
mm_init()
{
    dbg_uart_str("mm_init\n");

    kmm = (MemoryManagement*)(DIRECT_BASE);
    dbg_uart_str("  Heap point: ");
    dbg_uart_hex(kmm->heap_point);
    dbg_uart_str("  Page ent allocated: ");
    dbg_uart_hex(kmm->ent_allocated);
    dbg_uart_str("\n");

    // map buddy tree space to 1MB phy space after kernel
    _mm_map_to_physical(
            kmm,
            (kmm->heap_point + MAPPED_BASE + MM_PAGE_SIZE - 1) >> MM_PAGE_SHIFT,
            BUDDY_TREE_SIZE >> MM_PAGE_SHIFT,
            (kmm->heap_point + MM_PAGE_SIZE - 1) >> MM_PAGE_SHIFT,
            true
        );
    mm_buddy_tree = (uint32_t*)(MAPPED_BASE + kmm->heap_point);
}

/**
 * map @page_nr pages virtual address from @v_start into physical frames start
 * from @p_start
 *
 * @param mm the struct to operate with
 * @param v_start starting page number of virtual address
 * @param page_nr number of pages to be mapped
 * @param p_start starting page number of physical address
 * @param we write enable for those pages
 */
static void
_mm_map_to_physical(
        MemoryManagement *mm,
        page_t v_start,
        size_t page_nr,
        page_t p_start,
        bool we
    )
{
    while (page_nr) {
        MMPageDir *dir = &mm->dir[_mm_get_page_dir_from_page(v_start)];
        if (!dir->valid) {
            dir->valid = 1;
            dir->frame = bm_alloc(mm->dir_bitmap, bm_limit(mm->dir_bitmap));

            // TODO check enough space of frame
        }

        MMPageEnt *ent = (MMPageEnt*)mm_get_direct_mapped_by_frame(dir->frame);

        size_t fill_nr = minu32(page_nr, ENT_PER_PAGE - (v_start & (ENT_PER_PAGE - 1)));

        _mm_fill_page_ent(
                ent + (v_start & (ENT_PER_PAGE - 1)),
                fill_nr,
                p_start,
                we
            );

        v_start += fill_nr;
        page_nr -= fill_nr;
        p_start += fill_nr;
    }
}

/**
 * fill @page_nr page entries start from @ent with physical frames start from
 * @p_start
 *
 * @param ent starting page entries to fill
 * @param page_nr number of pages to fill
 * @param p_start starting physical frame number 
 */
static void
_mm_fill_page_ent(
        MMPageEnt *ent,
        size_t page_nr,
        page_t p_start,
        bool we
    )
{
    while (--page_nr) {
        ent->valid  = 1;
        ent->we     = we;
        ent->frame  = p_start;

        ++ent;
        ++p_start;
    }
}

static inline addr_t
_mm_get_page_dir(addr_t addr)
{ return addr >> 22; }

static inline addr_t
_mm_get_page_ent(addr_t addr)
{ return (addr >> 12) & 0x3FF; }

static inline page_t
_mm_get_page_no(addr_t addr)
{ return (_mm_get_page_dir(addr) << 10) | _mm_get_page_ent(addr); }

static inline addr_t
_mm_get_page_dir_from_page(page_t page)
{ return page >> 10; }
