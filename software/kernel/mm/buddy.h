#ifndef _COS_BUDDY_H_
#define _COS_BUDDY_H_

#include <stdint.h>

#include "mm.h"

void mm_buddy_init();
page_t mm_buddy_alloc(size_t);
void mm_buddy_free(page_t, size_t);

#endif // _COS_BUDDY_H_
