#include <CuTest.h>
#include <stdlib.h>

#include "mm/slab.h"

int posix_memalign(void **, size_t, size_t);

void *
_slab_test_page_alloc()
{
    void *retval;
    if (posix_memalign(&retval, PAGE_SIZE, PAGE_SIZE)) {
        retval = NULL;
    }
    return retval;
}

void
_slab_test_page_free(void *ptr)
{ free(ptr); }

void
mm_slab_init_test(CuTest *tc)
{
    Slab uut;

    mm_slab_init(&uut, _slab_test_page_alloc, _slab_test_page_free);

    for (int i = 0; i < SLAB_SHIFT; ++i) {
        CuAssertIntEquals(tc, 0, list_size(uut.pages + i));
    }
}

void
mm_slab_alloc_test(CuTest *tc)
{
    Slab uut;

    mm_slab_init(&uut, _slab_test_page_alloc, _slab_test_page_free);

    int *alloc_res_int = mm_slab_alloc(&uut, sizeof(int));
    CuAssertIntEquals(tc,
            (PAGE_SIZE - sizeof(SlabPage)) / 16 - 1,
            list_size(&list_get(list_head(uut.pages), SlabPage, _link)->free_list)
        );

    mm_slab_free(&uut, alloc_res_int);
    CuAssertIntEquals(tc, 1, list_size(uut.pages));
}

CuSuite *
mm_slab_test_suite(void)
{
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, mm_slab_init_test);
    SUITE_ADD_TEST(suite, mm_slab_alloc_test);

    return suite;
}
