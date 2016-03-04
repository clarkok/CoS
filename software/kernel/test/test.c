#include <CuTest.h>
#include <stdio.h>

#include "test/utils/linked-list-test.c"
#include "test/utils/sb-tree-test.c"

int
main()
{
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();

    CuSuiteAddSuite(suite, linked_list_test_suite());
    CuSuiteAddSuite(suite, sb_tree_test_suit());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);

    printf("%s\n", output->buffer);

    return 0;
}
