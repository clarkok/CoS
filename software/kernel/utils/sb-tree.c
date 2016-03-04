#include <assert.h>

#include "sb-tree.h"

#define _sb_size_could_null(node)  ((node) ? (node)->size : 0)

static inline void
_sb_update_count(SBNode *n)
{
    n->size = 1 + _sb_size_could_null(n->left)
                + _sb_size_could_null(n->right);
}

static inline SBNode *
_sb_turn_left(SBNode *n)
{
    /**
     *      N        R
     *       \      /
     *        R -> N
     *       /      \
     *      L        L
     */

    assert(n->right);

    SBNode *p = n->parent,
               *r = n->right,
               *l = r->left;

    if (p) {
        if (sb_is_left(n)) {
            p->left = r;
        }
        else {
            p->right = r;
        }
    }

    n->right = l;
    n->parent = r;
    if (l) l->parent = n;
    r->left = n;
    r->parent = p;

    _sb_update_count(n);
    _sb_update_count(r);

    return r;
}

static inline SBNode *
_sb_turn_right(SBNode *n)
{
    /**
     *      N       L
     *     /         \
     *    L     ->    N
     *     \         /
     *      R       R
     */

    assert(n->left);

    SBNode *p = n->parent,
               *l = n->left,
               *r = l->right;

    if (p) {
        if (sb_is_left(n)) {
            p->left = l;
        }
        else {
            p->right = l;
        }
    }

    n->left = r;
    n->parent = l;
    if (r) r->parent = n;
    l->right = n;
    l->parent = p;

    _sb_update_count(n);
    _sb_update_count(l);

    return l;
}

void
sb_rebalance(SBNode *node)
{
    if (!node) return;

    SBTree *tree = node->tree;

    while (1) {
        size_t l_size = _sb_size_could_null(node->left);
        size_t r_size = _sb_size_could_null(node->right);
        node->size = l_size + r_size + 1;

        if (l_size > r_size + 1) {
            node = _sb_turn_right(node);
        }
        else if (r_size > l_size + 1) {
            node = _sb_turn_left(node);
        }

        if (node->parent) node = node->parent;
        else {
            tree->root = node;
            return;
        }
    }
}

SBNode *
sb_unlink(SBNode *node)
{
    if (node->left) {
        if (node->right) {
            SBNode *replacement = sb_unlink(sb_rightmost(node->left));
            sb_replace(node, replacement);
            return node;
        }
        else {
            node->left->parent = node->parent;
            if (node->parent) {
                if (sb_is_left(node)) {
                    node->parent->left = node->left;
                }
                else {
                    node->parent->right = node->left;
                }
                sb_rebalance(node->parent);
            }
            else {
                node->tree->root = node->left;
            }

            --node->tree->size;
            return node;
        }
    }
    else {
        if (node->right) node->right->parent = node->parent;
        if (node->parent) {
            if (sb_is_left(node)) {
                node->parent->left = node->right;
            }
            else {
                node->parent->right = node->right;
            }
            sb_rebalance(node->parent);
        }
        else {
            node->tree->root = node->right;
        }

        --node->tree->size;
        return node;
    }

    assert(0);      // should not reach here
}
