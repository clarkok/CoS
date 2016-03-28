#include <CuTest.h>
#include <stdlib.h>

#include "mm/linked-buddy.h"

#define MM_LINKED_BUDDY_TEST_PAGE_NR    (2ull * 1024 * 1024 * 1024 / PAGE_SIZE) 

void
mm_linked_buddy_new_test(CuTest *tc)
{
    LinkedBuddy *buddy = mm_linked_buddy_new(MM_LINKED_BUDDY_TEST_PAGE_NR);
    
    CuAssertTrue(tc, !!buddy);
    CuAssertIntEquals(tc, 31 - 12 + 1, buddy->level_nr);

    mm_linked_buddy_destroy(buddy);
}

void
mm_linked_buddy_alloc_test(CuTest *tc)
{
    LinkedBuddy *buddy = mm_linked_buddy_new(MM_LINKED_BUDDY_TEST_PAGE_NR);

    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR, buddy->free_nr);

    int alloc_1 = mm_linked_buddy_alloc(buddy, 1);
    CuAssertIntEquals(tc, 0, alloc_1);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 1, buddy->free_nr);

    int alloc_2 = mm_linked_buddy_alloc(buddy, 2);
    CuAssertIntEquals(tc, 2, alloc_2);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 3, buddy->free_nr);

    int alloc_3 = mm_linked_buddy_alloc(buddy, 3);
    CuAssertIntEquals(tc, 4, alloc_3);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 7, buddy->free_nr);

    int alloc_4 = mm_linked_buddy_alloc(buddy, 4);
    CuAssertIntEquals(tc, 8, alloc_4);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 11, buddy->free_nr);

    mm_linked_buddy_destroy(buddy);
}

void
mm_linked_buddy_free_test(CuTest *tc)
{
    LinkedBuddy *buddy = mm_linked_buddy_new(MM_LINKED_BUDDY_TEST_PAGE_NR);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR, buddy->free_nr);

    int alloc_1 = mm_linked_buddy_alloc(buddy, 1);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 1, buddy->free_nr);

    int alloc_2 = mm_linked_buddy_alloc(buddy, 2);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 3, buddy->free_nr);

    int alloc_3 = mm_linked_buddy_alloc(buddy, 3);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 7, buddy->free_nr);

    int alloc_4 = mm_linked_buddy_alloc(buddy, 4);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 11, buddy->free_nr);

    mm_linked_buddy_free(buddy, alloc_1);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 10, buddy->free_nr);
    mm_linked_buddy_free(buddy, alloc_2);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 8, buddy->free_nr);
    mm_linked_buddy_free(buddy, alloc_3);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 4, buddy->free_nr);
    mm_linked_buddy_free(buddy, alloc_4);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR, buddy->free_nr);

    alloc_1 = mm_linked_buddy_alloc(buddy, 1);
    CuAssertIntEquals(tc, 0, alloc_1);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 1, buddy->free_nr);

    alloc_2 = mm_linked_buddy_alloc(buddy, 2);
    CuAssertIntEquals(tc, 2, alloc_2);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 3, buddy->free_nr);

    alloc_3 = mm_linked_buddy_alloc(buddy, 3);
    CuAssertIntEquals(tc, 4, alloc_3);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 7, buddy->free_nr);

    alloc_4 = mm_linked_buddy_alloc(buddy, 4);
    CuAssertIntEquals(tc, 8, alloc_4);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 11, buddy->free_nr);

    mm_linked_buddy_free(buddy, alloc_1);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 10, buddy->free_nr);
    mm_linked_buddy_free(buddy, alloc_2);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 8, buddy->free_nr);
    mm_linked_buddy_free(buddy, alloc_3);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 4, buddy->free_nr);
    mm_linked_buddy_free(buddy, alloc_4);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR, buddy->free_nr);

    int alloc = mm_linked_buddy_alloc(buddy, 524288);
    CuAssertIntEquals(tc, 0, alloc);
    CuAssertIntEquals(tc, 0, buddy->free_nr);

    mm_linked_buddy_destroy(buddy);
}

void
mm_linked_buddy_alloc_hint_test(CuTest *tc)
{
    LinkedBuddy *buddy = mm_linked_buddy_new(MM_LINKED_BUDDY_TEST_PAGE_NR);

    int alloc_1 = mm_linked_buddy_alloc(buddy, 1);
    CuAssertIntEquals(tc, 0, alloc_1);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 1, buddy->free_nr);

    int alloc_2 = mm_linked_buddy_alloc(buddy, 2);
    CuAssertIntEquals(tc, 2, alloc_2);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 3, buddy->free_nr);

    int alloc_3 = mm_linked_buddy_alloc(buddy, 3);
    CuAssertIntEquals(tc, 4, alloc_3);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 7, buddy->free_nr);

    int alloc_4 = mm_linked_buddy_alloc(buddy, 4);
    CuAssertIntEquals(tc, 8, alloc_4);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 11, buddy->free_nr);

    int alloc_hint_1 = mm_linked_buddy_alloc_hint(buddy, 1, MM_LINKED_BUDDY_TEST_PAGE_NR - 1);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 1, alloc_hint_1);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 12, buddy->free_nr);

    int alloc_hint_2 = mm_linked_buddy_alloc_hint(buddy, 2, MM_LINKED_BUDDY_TEST_PAGE_NR - 4);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 4, alloc_hint_2);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 14, buddy->free_nr);

    int alloc_hint_3 = mm_linked_buddy_alloc_hint(buddy, 3, MM_LINKED_BUDDY_TEST_PAGE_NR - 4);
    CuAssertIntEquals(tc, -1, alloc_hint_3);
    CuAssertIntEquals(tc, MM_LINKED_BUDDY_TEST_PAGE_NR - 14, buddy->free_nr);

    mm_linked_buddy_destroy(buddy);
}

CuSuite *
mm_linked_buddy_test_suite(void)
{
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, mm_linked_buddy_new_test);
    SUITE_ADD_TEST(suite, mm_linked_buddy_alloc_test);
    SUITE_ADD_TEST(suite, mm_linked_buddy_free_test);
    SUITE_ADD_TEST(suite, mm_linked_buddy_alloc_hint_test);

    return suite;
}
