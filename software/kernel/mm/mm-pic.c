#include "core/kernel.h"
#include "mm.h"

void
pic_mm_init()
{
    size_t kernel_size = *(size_t*)(0xC0000000) + (1 * 1024 * 1024);

    PageDir *pdir_ptr = (PageDir*)(0xC0001000);
    pdir_ptr[0].valid = 1;
    pdir_ptr[0].page_ent = 2;

    PageEnt *pent_ptr = (PageEnt*)(0xC0002000);

    for (size_t page = 0; page < (kernel_size + PAGE_SIZE - 1) / PAGE_SIZE; ++page, ++pent_ptr) {
        pent_ptr->valid = 1;
        pent_ptr->we = 1;
        pent_ptr->page = page;
    }

    // stack section
    pdir_ptr[1023].valid = 1;
    pdir_ptr[1023].page_ent = 3;

    pent_ptr = (PageEnt*)(0xC0003000);
    pent_ptr[1023].valid = 1;
    pent_ptr[1023].we = 1;
    pent_ptr[1023].page = (kernel_size + PAGE_SIZE - 1) / PAGE_SIZE;

    out_ptb((size_t)pdir_ptr);
}
