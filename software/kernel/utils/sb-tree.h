#ifndef _COS_SB_TREE_H_
#define _COS_SB_TREE_H_

#include <stddef.h>

#include "core/cos.h"

typedef struct SBNode
{
    struct SBNode *left;
    struct SBNode *right;
    struct SBNode *parent;
    struct SBTree *tree;
    size_t size;
} SBNode;

typedef struct SBTree
{
    struct SBNode *root;
    size_t size;
} SBTree;

#define sb_init(tree)           \
    do {                        \
        (tree)->root = NULL;    \
        (tree)->size = 0;       \
    } while (0)

#define sb_get(ptr, type, mem)      container_of(ptr, type, mem)
#define sb_from_node(node)          ((node)->tree)
#define sb_node_linked(node)        (sb_from_node(node))
#define sb_node_init(node)          ((node)->tree = NULL)
#define SB_TREE_INIT                {NULL, 0}

#define sb_head(tree)       (sb_leftmost((tree)->root))
#define sb_tail(tree)       (sb_rightmost((tree)->root))

#define sb_size(tree)       ((tree)->size)
#define sb_root(tree)       ((tree)->root)

#define sb_is_leaf(node)    ((node)->left || (node)->right)
#define sb_is_left(node)    ((node) == ((node)->parent->left))

#define sb_for_each(tree, n)    \
    for (SBNode *n = sb_head(tree); n; n = sb_next(n))

#define sb_for_each_r(tree, n)    \
    for (SBNode *n = sb_tail(tree); n; n = sb_prev(n))

void sb_rebalance(SBNode *node);

static inline SBNode *
sb_link(SBNode *node, SBNode *parent, SBNode **link, SBTree *tree)
{
    node->left = node->right = NULL;
    node->parent = parent;
    node->tree = tree;
    node->size = 1;
    *link = node;
    sb_rebalance(parent);
    tree->size ++;
    return node;
}

static inline SBNode *
sb_leftmost(SBNode *node)
{
    if (!node) return NULL;
    while (node->left) {
        node = node->left;
    }
    return node;
}

static inline SBNode *
sb_rightmost(SBNode *node)
{
    if (!node) return NULL;
    while (node->right) {
        node = node->right;
    }
    return node;
}

static inline SBNode *
sb_prev_ancestor(SBNode *node)
{
    while (node->parent && sb_is_left(node)) {
        node = node->parent;
    }
    return node->parent;
}

static inline SBNode *
sb_next_ancestor(SBNode *node)
{
    while (node->parent && !sb_is_left(node)) {
        node = node->parent;
    }
    return node->parent;
}

static inline SBNode *
sb_prev(SBNode *node)
{
    if (node->left) {
        return sb_rightmost(node->left);
    }

    if (node->parent && sb_is_left(node)) {
        return sb_prev_ancestor(node);
    }

    return node->parent;
}

static inline SBNode *
sb_next(SBNode *node)
{
    if (node->right) {
        return sb_leftmost(node->right);
    }

    if (node->parent && !sb_is_left(node)) {
        return sb_next_ancestor(node);
    }

    return node->parent;
}

static inline void
sb_replace(SBNode *dst, SBNode *src)
{
    src->tree = dst->tree;
    src->parent = dst->parent;
    src->left = dst->left;
    src->right = dst->right;
    src->size = dst->size;

    if (src->parent) {
        if (sb_is_left(dst)) {
            src->parent->left = src;
        }
        else {
            src->parent->right = src;
        }
    }
    else {
        src->tree->root = src;
    }

    if (src->left)  src->left->parent = src;
    if (src->right) src->right->parent = src;
}

SBNode *sb_unlink(SBNode *node);

#endif
