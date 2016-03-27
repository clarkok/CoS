#include <CuTest.h>
#include <stdio.h>

#include "test/utils/linked-list-test.c"
#include "test/utils/sb-tree-test.c"
#include "test/utils/bits-test.c"
#include "test/mm/buddy-test.c"
#include "test/mm/slab-test.c"
#include "test/mm/linked-buddy-test.c"

int
main()
{
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();

    CuSuiteAddSuite(suite, linked_list_test_suite());
    CuSuiteAddSuite(suite, sb_tree_test_suite());
    CuSuiteAddSuite(suite, bits_test_suite());
    CuSuiteAddSuite(suite, mm_buddy_test_suite());
    CuSuiteAddSuite(suite, mm_slab_test_suite());
    CuSuiteAddSuite(suite, mm_linked_buddy_test_suite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);

    printf("%s\n", output->buffer);

    return 0;
}
