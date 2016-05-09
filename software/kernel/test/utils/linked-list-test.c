#include <CuTest.h>
#include <stdlib.h>

#include "utils/linked-list.h"

typedef struct LinkedListTestNode
{
    LinkedNode _link;
    int number;
} LinkedListTestNode;

void
linked_list_append_test(CuTest *tc)
{
    LinkedList uut;
    list_init(&uut);

    for (int i = 0; i < 16; ++i) {
        LinkedListTestNode *node = (LinkedListTestNode*)malloc(sizeof(LinkedListTestNode));
        node->number = i;
        list_append(&uut, &node->_link);
    }

    CuAssertIntEquals(tc, 16, list_size(&uut));

    int i = 0;
    list_for_each(&uut, n) {
        CuAssertIntEquals(tc, i++, list_get(n, LinkedListTestNode, _link)->number);
    }
    i = 15;
    list_for_each_r(&uut, n) {
        CuAssertIntEquals(tc, i--, list_get(n, LinkedListTestNode, _link)->number);
    }
}

void
linked_list_prepend_test(CuTest *tc)
{
    LinkedList uut;
    list_init(&uut);

    for (int i = 0; i < 16; ++i) {
        LinkedListTestNode *node = (LinkedListTestNode*)malloc(sizeof(LinkedListTestNode));
        node->number = i;
        list_prepend(&uut, &node->_link);
        CuAssertTrue(tc, (int)node->_link.prev);
    }

    CuAssertIntEquals(tc, 16, list_size(&uut));

    int i = 15;
    list_for_each(&uut, n) {
        CuAssertIntEquals(tc, i--, list_get(n, LinkedListTestNode, _link)->number);
    }
    i = 0;
    list_for_each_r(&uut, n) {
        CuAssertIntEquals(tc, i++, list_get(n, LinkedListTestNode, _link)->number);
    }
}

void
linked_list_before_test(CuTest *tc)
{
    LinkedList uut;
    list_init(&uut);

    LinkedListTestNode *last = (LinkedListTestNode*)malloc(sizeof(LinkedListTestNode));
    last->number = 0;
    list_append(&uut, &last->_link);

    for (int i = 1; i < 16; ++i) {
        LinkedListTestNode *node = (LinkedListTestNode*)malloc(sizeof(LinkedListTestNode));
        node->number = i;
        list_before(&last->_link, &node->_link);
        last = node;
    }

    CuAssertIntEquals(tc, 16, list_size(&uut));

    int i = 15;
    list_for_each(&uut, n) {
        CuAssertIntEquals(tc, i--, list_get(n, LinkedListTestNode, _link)->number);
    }
    i = 0;
    list_for_each_r(&uut, n) {
        CuAssertIntEquals(tc, i++, list_get(n, LinkedListTestNode, _link)->number);
    }
}

void
linked_list_after_test(CuTest *tc)
{
    LinkedList uut;
    list_init(&uut);

    LinkedListTestNode *last = (LinkedListTestNode*)malloc(sizeof(LinkedListTestNode));
    last->number = 0;
    list_append(&uut, &last->_link);

    for (int i = 1; i < 16; ++i) {
        LinkedListTestNode *node = (LinkedListTestNode*)malloc(sizeof(LinkedListTestNode));
        node->number = i;
        list_after(&last->_link, &node->_link);
        last = node;
    }

    CuAssertIntEquals(tc, 16, list_size(&uut));

    int i = 0;
    list_for_each(&uut, n) {
        CuAssertIntEquals(tc, i++, list_get(n, LinkedListTestNode, _link)->number);
    }
    i = 15;
    list_for_each_r(&uut, n) {
        CuAssertIntEquals(tc, i--, list_get(n, LinkedListTestNode, _link)->number);
    }
}

void
linked_list_unlink_test(CuTest *tc)
{
    LinkedList uut;
    list_init(&uut);

    for (int i = 0; i < 16; ++i) {
        LinkedListTestNode *node = (LinkedListTestNode*)malloc(sizeof(LinkedListTestNode));
        node->number = i;
        list_append(&uut, &node->_link);
    }

    LinkedNode *unlinked;
    CuAssertIntEquals(tc, 16, list_size(&uut));

    unlinked = list_unlink(list_head(&uut));
    CuAssertIntEquals(tc, 0, list_get(unlinked, LinkedListTestNode, _link)->number);

    unlinked = list_unlink(list_tail(&uut));
    CuAssertIntEquals(tc, 15, list_get(unlinked, LinkedListTestNode, _link)->number);

    CuAssertIntEquals(tc, 14, list_size(&uut));
}

void
linked_list_move_test(CuTest *tc)
{
    LinkedList uut_a;
    LinkedList uut_b;

    list_init(&uut_a);

    for (int i = 0; i < 16; ++i) {
        LinkedListTestNode *node = (LinkedListTestNode*)malloc(sizeof(LinkedListTestNode));
        node->number = i;
        list_append(&uut_a, &node->_link);
    }

    list_for_each(&uut_a, node) {
        CuAssertTrue(tc, list_from_node(node) == &uut_a);
    }

    CuAssertIntEquals(tc, 16, list_size(&uut_a));

    list_move(&uut_b, &uut_a);

    CuAssertIntEquals(tc, 0, list_size(&uut_a));
    CuAssertIntEquals(tc, 16, list_size(&uut_b));

    list_for_each(&uut_a, node) {
        CuAssertTrue(tc, list_from_node(node) == &uut_a);
    }

    list_for_each(&uut_b, node) {
        CuAssertTrue(tc, list_from_node(node) == &uut_b);
    }
}

CuSuite *
linked_list_test_suite(void)
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, linked_list_append_test);
    SUITE_ADD_TEST(suite, linked_list_prepend_test);
    SUITE_ADD_TEST(suite, linked_list_before_test);
    SUITE_ADD_TEST(suite, linked_list_after_test);
    SUITE_ADD_TEST(suite, linked_list_unlink_test);
    SUITE_ADD_TEST(suite, linked_list_move_test);

    return suite;
}
