#include <CuTest.h>
#include <stdlib.h>

#include "utils/sb-tree.h"

typedef struct SBTreeTestNode
{
    SBNode _node;
    int number;
} SBTreeTestNode;

void
_sb_tree_test_insert(SBTree *tree, int number)
{
    SBNode **ptr = &sb_root(tree),
           *parent = NULL;

    while (*ptr) {
        parent = *ptr;
        if (sb_get(*ptr, SBTreeTestNode, _node)->number > number) {
            ptr = &(*ptr)->left;
        }
        else {
            ptr = &(*ptr)->right;
        }
    }

    SBTreeTestNode *node = (SBTreeTestNode*)malloc(sizeof(SBTreeTestNode));
    node->number = number;
    sb_link(&node->_node, parent, ptr, tree);
}

SBNode *
_sb_tree_test_find(SBTree *tree, int number)
{
    SBNode *node = sb_root(tree);

    while (node) {
        if (sb_get(node, SBTreeTestNode, _node)->number == number) return node;
        if (sb_get(node, SBTreeTestNode, _node)->number > number) {
            node = node->left;
        }
        else {
            node = node->right;
        }
    }
    return node;
}

void
sb_tree_insert_test(CuTest *tc)
{
    SBTree uut = SB_TREE_INIT;

    for (int i = 0; i < 16; i++) {
        _sb_tree_test_insert(&uut, i);

        CuAssertIntEquals(tc, i+1, sb_size(&uut));
        CuAssertIntEquals(tc, sb_size(&uut), sb_root(&uut)->size);

        for (int j = 0; j <= i; j++) {
            CuAssertTrue(tc, (NULL != _sb_tree_test_find(&uut, j)));
            CuAssertIntEquals(tc, j, sb_get(_sb_tree_test_find(&uut, j), SBTreeTestNode, _node)->number);
        }

        int j = 0;
        sb_for_each(&uut, n) {
            CuAssertIntEquals(tc, j++, sb_get(n, SBTreeTestNode, _node)->number);
        }
        CuAssertIntEquals(tc, j, i+1);
        sb_for_each_r(&uut, n) {
            CuAssertIntEquals(tc, --j, sb_get(n, SBTreeTestNode, _node)->number);
        }
    }
}

void
sb_tree_unlink_test(CuTest *tc)
{
    SBTree uut = SB_TREE_INIT;

    for (int i = 0; i < 16; i++) {
        _sb_tree_test_insert(&uut, i);
    }

    CuAssertIntEquals(tc, 16, sb_size(&uut));

    SBNode *head = sb_unlink(sb_head(&uut));
    CuAssertIntEquals(tc, 0, sb_get(head, SBTreeTestNode, _node)->number);

    SBNode *tail = sb_unlink(sb_tail(&uut));
    CuAssertIntEquals(tc, 15, sb_get(tail, SBTreeTestNode, _node)->number);

    CuAssertIntEquals(tc, 14, sb_size(&uut));

    int j = 1;
    sb_for_each(&uut, n) {
        CuAssertIntEquals(tc, j++, sb_get(n, SBTreeTestNode, _node)->number);
    }
}

CuSuite *
sb_tree_test_suite(void)
{
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, sb_tree_insert_test);
    SUITE_ADD_TEST(suite, sb_tree_unlink_test);

    return suite;
}
