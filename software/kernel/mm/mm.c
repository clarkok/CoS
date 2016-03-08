#include "mm.h"

static PageGroup *
_page_group_insert(SBTree *tree, PageGroup *node)
{
    SBNode **link = &sb_root(tree),
           *parent = NULL;

    while (*link) {
        parent = *link;
        if (sb_get(*link, PageGroup, _node)->v_page_start > node->v_page_start) {
            link = &(*link)->left;
        }
        else {
            link = &(*link)->right;
        }
    }

    sb_link(&node->_node, parent, link, tree);
}

static PageGroup *
_page_group_find(SBTree *tree, size_t v_page)
{
    SBNode *ptr = sb_root(tree);

    while (ptr) {
        PageGroup *node = sb_get(ptr, PageGroup, _node);
        if (node->v_page_start == v_page) return node;
        if (node->v_page_start > v_page)    ptr = ptr->left;
        else                                ptr = ptr->right;
    }

    return NULL;
}

