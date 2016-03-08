#include <CuTest.h>
#include <stdlib.h>

#include "utils/bits.h"

void
bits_lsb_idx_32_test(CuTest *tc)
{
    uint32_t i = 0xFFFFFFFF;
    int j = 0;

    while (i) {
        CuAssertIntEquals(tc, j++, bits_lsb_idx_32(i));
        i <<= 1;
    }
}

CuSuite *
bits_test_suite(void)
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, bits_lsb_idx_32_test);

    return suite;
}
