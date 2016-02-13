#ifndef _COS_BITMAP_H_
#define _COS_BITMAP_H_

#include <stddef.h>

#include "utils.h"

typedef uint32_t bitmap_t;

#define bm_limit(bm)    \
    (bm + array_item_count(bm))

size_t bm_alloc(bitmap_t *, bitmap_t *);

static inline void
bm_set(bitmap_t *bitmap, size_t idx)
{ *(bitmap + (idx >> 5)) |= (1 << (idx & 31)); }

static inline void
bm_unset(bitmap_t *bitmap, size_t idx)
{ *(bitmap + (idx >> 5)) &= ~(1 << (idx & 31)); }

#endif // _COS_BITMAP_H_
