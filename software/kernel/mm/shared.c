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

#define shared_dump()                                   \
    do {                                                \
        kprintf("SHAREDDump:\n");                       \
        sb_for_each(&mm_shared_page_tree, node) {       \
            SharedPages *sp =                           \
                sb_get(node, SharedPages, _node);       \
            kprintf(                                    \
                    "SHARED     page: 0x%x,\t"          \
                    "count: 0x%x,\t"                    \
                    "ref: 0x%x,\t"                      \
                    "this: 0x%x,\t"                     \
                    "left: 0x%x,\t"                     \
                    "right: 0x%x\t"                     \
                    "parent: 0x%x\t"                    \
                    "size: 0x%x\n"                      \
                    ,                                   \
                    sp->p_page_start,                   \
                    sp->page_count,                     \
                    sp->ref_count,                      \
                    &sp->_node,                         \
                    sp->_node.left,                     \
                    sp->_node.right,                    \
                    sp->_node.parent,                   \
                    sp->_node.size                      \
                );                                      \
        }                                               \
        kprintf("SHARED\n");                            \
    } while (0)


SharedPages *
mm_shared_add_ref(size_t p_page_start, size_t page_count, int cow)
{
    kprintf("\nSHARED: add ref 0x%x, 0x%x\n", p_page_start, page_count);

    SharedPages *pages = mm_shared_lookup(p_page_start);
    if (pages) {
        assert(pages->page_count == page_count);
        pages->ref_count++;

        shared_dump();

        return pages;
    }
    else {
        SharedPages *new_pages = (SharedPages*)kmalloc(sizeof(SharedPages));
        new_pages->p_page_start = p_page_start;
        new_pages->page_count = page_count;
        new_pages->ref_count = 1;
        new_pages->copy_on_write = cow;

        _mm_shared_pages_insert(&mm_shared_page_tree, new_pages);

        shared_dump();

        return new_pages;
    }
}

int
mm_shared_rm_ref(SharedPages *shared_pages)
{
    kprintf("\nSHARED: rm ref 0x%x, 0x%x\n", shared_pages->p_page_start, shared_pages->page_count);

    if (!--(shared_pages->ref_count)) {
        sb_unlink(&shared_pages->_node);
        kfree(shared_pages);

        shared_dump();

        return 0;
    }

    shared_dump();

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
