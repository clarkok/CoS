#include "bitmap.h"

size_t
bm_alloc(bitmap_t *bitmap, bitmap_t *limit)
{
    size_t ret = 0;
    while (bitmap != limit) {
        if (~*bitmap) {
            size_t idx_in_u32 = get_lsb_idx(~*bitmap);
            *bitmap |= (1 << idx_in_u32);
            return ret + idx_in_u32;
        }
        ret += 32;
    }
    return 0;
}

