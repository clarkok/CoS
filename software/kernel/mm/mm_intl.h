#ifndef _COS_MM_INTL_H_
#define _COS_MM_INTL_H_

#include "mm.h"

void _mm_map_to_physical(MemoryManagement *, addr_t, size_t, addr_t, bool);
void _mm_fill_page_ent(MMPageEnt *, size_t, addr_t, bool);

addr_t _mm_get_page_dir(addr_t) __ALWAYS_INLINE__;
addr_t _mm_get_page_ent(addr_t) __ALWAYS_INLINE__;
page_t _mm_get_page_no(addr_t) __ALWAYS_INLINE__;
addr_t _mm_get_page_dir_from_page(page_t) __ALWAYS_INLINE__;

extern MemoryManagement *_kmm;

#endif // _COS_MM_INTL_H_
