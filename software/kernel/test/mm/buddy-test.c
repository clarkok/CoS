#include <CuTest.h>
#include <stdlib.h>

#include "mm/buddy.h"

void
mm_buddy_init_test(CuTest *tc)
{
    Buddy uut;
    uut.tree = malloc(MM_BUDDY_TREE_SIZE);

    mm_buddy_init(&uut, 0);
    CuAssertIntEquals(tc, (1 << (MM_BUDDY_SHIFT - 1)), uut.free_nr);

    mm_buddy_init(&uut, 10);
    CuAssertIntEquals(tc, (1 << (MM_BUDDY_SHIFT - 1)) - 16, uut.free_nr);

    mm_buddy_init(&uut, 16);
    CuAssertIntEquals(tc, (1 << (MM_BUDDY_SHIFT - 1)) - 16, uut.free_nr);
}

void
mm_buddy_alloc_test(CuTest *tc)
{
    Buddy uut;
    uut.tree = malloc(MM_BUDDY_TREE_SIZE);

    int total = (1 << (MM_BUDDY_SHIFT - 1));

    mm_buddy_init(&uut, 0);

    int alloc_res_10 = mm_buddy_alloc(&uut, 10);
    CuAssertIntEquals(tc, 0, alloc_res_10);
    CuAssertIntEquals(tc, total -= 16, uut.free_nr);

    int alloc_res_16 = mm_buddy_alloc(&uut, 16);
    CuAssertIntEquals(tc, 16, alloc_res_16);
    CuAssertIntEquals(tc, total -= 16, uut.free_nr);

    int alloc_res_30 = mm_buddy_alloc(&uut, 30);
    CuAssertIntEquals(tc, 32, alloc_res_30);
    CuAssertIntEquals(tc, total -= 32, uut.free_nr);

    mm_buddy_free(&uut, alloc_res_10);
    CuAssertIntEquals(tc, total += 16, uut.free_nr);
    mm_buddy_free(&uut, alloc_res_16);
    CuAssertIntEquals(tc, total += 16, uut.free_nr);

    int alloc_res_32 = mm_buddy_alloc(&uut, 32);
    CuAssertIntEquals(tc, 0, alloc_res_32);
    CuAssertIntEquals(tc, total -= 32, uut.free_nr);
}

CuSuite *
mm_buddy_test_suite(void)
{
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, mm_buddy_init_test);
    SUITE_ADD_TEST(suite, mm_buddy_alloc_test);

    return suite;
}
