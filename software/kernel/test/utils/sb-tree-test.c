#include <CuTest.h>
#include <stdio.h>
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

void
sb_tree_insert_many_times(CuTest *tc)
{
    SBTree uut = SB_TREE_INIT;

    for (int i = 0; i < 1000; ++i) {
        CuAssertIntEquals(tc, 0, sb_size(&uut));
        _sb_tree_test_insert(&uut, 0);
        CuAssertIntEquals(tc, 1, sb_size(&uut));
        free(sb_get(sb_unlink(sb_head(&uut)), SBTreeTestNode, _node));
        CuAssertIntEquals(tc, 0, sb_size(&uut));
    }
}

void
sb_tree_zigzag_test(CuTest *tc)
{
    SBTree uut = SB_TREE_INIT;

    _sb_tree_test_insert(&uut, 1);
    _sb_tree_test_insert(&uut, 3);
    _sb_tree_test_insert(&uut, 2);

    CuAssertIntEquals(tc, 2, sb_get(sb_root(&uut), SBTreeTestNode, _node)->number);
    CuAssertIntEquals(tc, 1, sb_get(sb_head(&uut), SBTreeTestNode, _node)->number);
    CuAssertIntEquals(tc, 3, sb_get(sb_tail(&uut), SBTreeTestNode, _node)->number);
}

void
sb_tree_rmroot_test(CuTest *tc)
{
    SBTree uut = SB_TREE_INIT;

    _sb_tree_test_insert(&uut, 2);
    _sb_tree_test_insert(&uut, 1);
    _sb_tree_test_insert(&uut, 3);

    CuAssertIntEquals(tc, 2, sb_get(sb_root(&uut), SBTreeTestNode, _node)->number);

    SBTreeTestNode *node = sb_get(sb_unlink(sb_root(&uut)), SBTreeTestNode, _node);

    CuAssertIntEquals(tc, 2, node->number);
    CuAssertIntEquals(tc, 2, sb_size(&uut));
    CuAssertIntEquals(tc, 1, sb_get(sb_head(&uut), SBTreeTestNode, _node)->number);
    CuAssertIntEquals(tc, 3, sb_get(sb_tail(&uut), SBTreeTestNode, _node)->number);
}

void
sb_tree_rm_to_empty(CuTest *tc)
{
    SBTree uut = SB_TREE_INIT;

    _sb_tree_test_insert(&uut, 1);
    _sb_tree_test_insert(&uut, 2);

    CuAssertIntEquals(tc, 2, sb_size(&uut));

    sb_unlink(_sb_tree_test_find(&uut, 1));
    sb_unlink(_sb_tree_test_find(&uut, 2));

    CuAssertIntEquals(tc, 0, sb_size(&uut));

    sb_for_each(&uut, node) {
        CuAssertTrue(tc, 0);
    }
}

typedef struct Node
{
    SBNode _node;

    unsigned key;
    int count;
} Node;

void
_sb_tree_test_insert_count(SBTree *tree, unsigned key, int count)
{
    SBNode **ptr = &sb_root(tree),
           *parent = NULL;

    while (*ptr) {
        parent = *ptr;
        if (sb_get(*ptr, Node, _node)->key > key) {
            ptr = &(*ptr)->left;
        }
        else {
            ptr = &(*ptr)->right;
        }
    }

    // SBTreeTestNode *node = (SBTreeTestNode*)malloc(sizeof(SBTreeTestNode));
    Node *node = (Node*)malloc(sizeof(Node));
    node->key = key;
    node->count = count;
    sb_link(&node->_node, parent, ptr, tree);
}

SBNode *
_sb_tree_test_find_count(SBTree *tree, unsigned key)
{
    SBNode *node = sb_root(tree);

    while (node) {
        if (sb_get(node, Node, _node)->key == key) return node;
        if (sb_get(node, Node, _node)->key > key) {
            node = node->left;
        }
        else {
            node = node->right;
        }
    }
    return node;
}

void
sb_tree_bug_test(CuTest *tc)
{
    static const int test_data[] = {
        +0x0812,
        +0x0812,
        -0x0812,
        -0x0812,
        +0x0812,
        +0x0812,
        +0x0817,
        -0x0812,
        -0x0817,
        -0x0812,
        +0x0812,
        +0x0812,
        +0x0817,
        +0x081e,
        +0x0817,
        +0x0817,
        -0x0812,
        +0x0817,
        +0x081e,
        -0x0812,
        -0x0817,
        -0x081e,
        -0x0817,
        -0x0817,
        +0x082b,
        -0x0817,
        +0x081e,
        -0x081e,
        +0x0812,
        +0x0812,
        -0x082b,
        -0x081e,
        -0x0812,
        +0x0833,
        +0x082b,
        -0x0812,
        +0x0841,
        -0x0833,
    };

    SBTree uut = SB_TREE_INIT;

    for (int i = 0; i < (int)sizeof(test_data) / (int)sizeof(test_data[0]); ++i) {
        if (test_data[i] > 0) {
            // printf(": add ref 0x%x\n", test_data[i]);
            Node *node = sb_get(_sb_tree_test_find_count(&uut, test_data[i]), Node, _node);
            if (node) {
                node->count++;
            }
            else {
                _sb_tree_test_insert_count(&uut, test_data[i], 1);
            }
        }
        else {
            Node *node = sb_get(_sb_tree_test_find_count(&uut, -test_data[i]), Node, _node);
            // printf(": rm ref 0x%x\n", -test_data[i]);
            CuAssertTrue(tc, !!node);
            CuAssertTrue(tc, (node->count) > 0);
            if (!--(node->count)) {
                free(sb_get(sb_unlink(&node->_node), Node, _node));
            }
        }

        /*
        printf("Dump:\n");
        sb_for_each(&uut, node) {
            CuAssertTrue(tc, !!node);
            Node *tn = sb_get(node, Node, _node);
            printf("    key: 0x%x\tcount: 0x%x\tthis:0x%x\tleft: 0x%x\tright: 0x%x\tparent: 0x%x\tsize: 0x%x\n",
                    tn->key, tn->count, (size_t)node, (size_t)node->left, (size_t)node->right, (size_t)node->parent, node->size
                );
        }
        printf("\n");
        */
    }
}

CuSuite *
sb_tree_test_suite(void)
{
    CuSuite *suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, sb_tree_insert_test);
    SUITE_ADD_TEST(suite, sb_tree_unlink_test);
    SUITE_ADD_TEST(suite, sb_tree_insert_many_times);
    SUITE_ADD_TEST(suite, sb_tree_zigzag_test);
    SUITE_ADD_TEST(suite, sb_tree_rmroot_test);
    SUITE_ADD_TEST(suite, sb_tree_rm_to_empty);
    SUITE_ADD_TEST(suite, sb_tree_bug_test);

    return suite;
}
