#include "mm.h"
#include "shared.h"

static SBTree mm_shared_page_tree;

static inline void
_mm_shared_pages_insert(SBTree *tree, SharedPages *node)
{
    SBNode **ptr = &sb_root(tree),
           *parent = NULL;

    while (*ptr) {
        parent = *ptr;
        SharedPages *shared_pages = sb_get(*ptr, SharedPages, _node);
        if (shared_pages->p_page_start > node->p_page_start) {
            ptr = &(*ptr)->left;
        }
        else {
            ptr = &(*ptr)->right;
        }
    }

    sb_link(&node->_node, parent, ptr, tree);
}

static inline
SharedPages *
_mm_shared_pages_find(SBTree *tree, size_t p_page_start)
{
    SBNode *node = sb_root(tree);

    while (node) {
        SharedPages *shared_pages = sb_get(node, SharedPages, _node);
        if (shared_pages->p_page_start == p_page_start) return shared_pages;
        if (shared_pages->p_page_start > p_page_start) {
            if (node->left) {
                node = node->left;
            }
            else if ((node = sb_prev(node))) {
                return sb_get(node, SharedPages, _node);
            }
            else {
                return NULL;
            }
        }
        else {
            if (node->right) {
                node = node->right;
            }
            else {
                return shared_pages;
            }
        }
    }
    return NULL;
}

void
mm_shared_init()
{ sb_init(&mm_shared_page_tree); }

SharedPages *
mm_shared_add_ref(size_t p_page_start, size_t page_count, int cow)
{
    SharedPages *new_pages = (SharedPages*)malloc(sizeof(SharedPages));
    new_pages->p_page_start = p_page_start;
    new_pages->page_count = page_count;
    new_pages->ref_count = 1;
    new_pages->copy_on_write = cow;

    _mm_shared_pages_insert(&mm_shared_page_tree, new_pages);

    return new_pages;
}

int
mm_shared_rm_ref(SharedPages *shared_pages)
{
    if (!--(shared_pages->ref_count)) {
        sb_unlink(&shared_pages->_node);
        free(shared_pages);
        return 0;
    }
    return shared_pages->ref_count;
}

SharedPages *
mm_shared_lookup(size_t p_page)
{
    SharedPages *shared_pages = _mm_shared_pages_find(&mm_shared_page_tree, p_page);
    if (!shared_pages) return NULL;

    if (p_page >= shared_pages->p_page_start &&
        p_page < shared_pages->p_page_start + shared_pages->page_count) {
        return shared_pages;
    }

    return NULL;
}
