#include <string.h>

#include "mm/mm.h"
#include "core/kernel.h"

#include "proc.h"

#define proc_kernel_thread(func)    (((size_t)(func)) + 1)

volatile ProcScene *proc_current_scene;
SBTree proc_tree;

static LinkedList proc_zero_queue;
static LinkedList proc_normal_queue;
static LinkedList proc_normal_noticks_queue;
static LinkedList proc_realtime_queue;

static size_t _proc_id;

static void
_proc_insert(SBTree *tree, Process *proc)
{
    SBNode **ptr = &sb_root(tree),
           *parent = NULL;

    while (*ptr) {
        parent = *ptr;
        if (sb_get(*ptr, Process, _node)->id > proc->id) {
            ptr = &(*ptr)->left;
        }
        else {
            ptr = &(*ptr)->right;
        }
    }
    sb_link(&proc->_node, parent, ptr, tree);
}

static Process *
_proc_find(SBTree *tree, size_t id)
{
    SBNode *node = sb_root(tree);

    while (node) {
        Process *proc = sb_get(node, Process, _node);
        if (proc->id == id) return proc;
        if (proc->id > id) {
            node = node->left;
        }
        else {
            node = node->right;
        }
    }
    return NULL;
}

static void
_proc_insert_into_queues(Process *proc)
{
    if (proc->priv_base >= PRIV_REALTIME) {
        Process *ptr = list_get(list_head(&proc_realtime_queue), Process, _link);
        while (ptr && ptr->priv_base >= proc->priv_base) {
            ptr = list_get(list_next(&ptr->_link), Process, _link);
        }
        if (ptr) {
            list_before(&ptr->_link, &proc->_link);
        }
        else {
            list_append(&proc_realtime_queue, &proc->_link);
        }
    }
    else if (proc->priv_base >= PRIV_NORMAL) {
        if (proc->ticks) {
            list_append(&proc_normal_queue, &proc->_link);
        }
        else {
            list_append(&proc_normal_noticks_queue, &proc->_link);
        }
    }
    else {
        list_append(&proc_zero_queue, &proc->_link);
    }
}

static void
init_proc()
{
    dbg_uart_str("init process running\n");

    for (;;) { }
}

static Process *
_proc_construct_init()
{
    Process *new_proc = malloc(sizeof(Process));
    new_proc->id = ++_proc_id;
    strncpy(new_proc->name, "init", PROC_NAME_LENGTH);
    new_proc->priv_base = PRIV_NORMAL;
    new_proc->priv_offset = 0;
    new_proc->ticks = 0;
    new_proc->state = PS_READY;

    mm_init_proc(&new_proc->mm);
    memset((void*)(&new_proc->scene), 0, sizeof(ProcScene));
    new_proc->scene.pc = proc_kernel_thread(init_proc);

    return new_proc;
}

void
proc_init()
{
    _proc_id = 0;

    sb_init(&proc_tree);
    list_init(&proc_zero_queue);
    list_init(&proc_normal_queue);
    list_init(&proc_normal_noticks_queue);
    list_init(&proc_realtime_queue);

    Process *init_proc = _proc_construct_init();
    _proc_insert(&proc_tree, init_proc);

    _proc_insert_into_queues(init_proc);

    proc_current_scene = &init_proc->scene;
}

void
proc_schedule()
{
    Process *proc_to_run = NULL;
    if (list_size(&proc_realtime_queue)) {
        proc_to_run = list_get(list_head(&proc_realtime_queue), Process, _link);
    }
    else if (list_size(&proc_normal_queue)) {
        if (list_from_node(&current_process->_link) == &proc_normal_queue) {
            proc_to_run = list_get(list_next(&current_process->_link), Process, _link);
        }
        else {
            proc_to_run = list_get(list_head(&proc_normal_queue), Process, _link);
        }

        // if process run out of ticks, move it to no-ticks queue
        if (!--(proc_to_run->ticks)) {
            list_append(&proc_normal_noticks_queue, list_unlink(&proc_to_run->_link));
        }
    }
    else if (list_size(&proc_normal_noticks_queue)) {
        while (list_size(&proc_normal_noticks_queue)) {
            Process *proc = list_get(list_unlink(list_head(&proc_normal_noticks_queue)), Process, _link);
            proc->ticks = proc->priv_base + proc->priv_offset;
            list_append(&proc_normal_queue, &proc->_link);
        }

        proc_to_run = list_get(list_head(&proc_normal_queue), Process, _link);

        // if process run out of ticks, move it to no-ticks queue
        if (!--(proc_to_run->ticks)) {
            list_append(&proc_normal_noticks_queue, list_unlink(&proc_to_run->_link));
        }
    }
    else {
        if (list_from_node(&current_process->_link) == &proc_zero_queue) {
            proc_to_run = list_get(list_next(&current_process->_link), Process, _link);
        }
        else {
            proc_to_run = list_get(list_head(&proc_zero_queue), Process, _link);
        }
    }

    assert(proc_to_run);

    if (proc_to_run != current_process) {
        mm_set_page_table(&proc_to_run->mm);
        proc_current_scene = &proc_to_run->scene;
    }
}

int
proc_do_fork()
{
    Process *current = current_process;
    Process *new_proc = malloc(sizeof(Process));

    new_proc->id = ++_proc_id;
    new_proc->state = PS_READY;
    strncpy(new_proc->name, current->name, PROC_NAME_LENGTH);
    new_proc->priv_base = current->priv_base;
    new_proc->priv_offset = 0;
    new_proc->ticks = current->ticks >> 1;
    current->ticks -= new_proc->ticks;

    mm_duplicate(&new_proc->mm, &current->mm);
    memcpy((void*)&new_proc->scene, (void*)&current->scene, sizeof(ProcScene));
    new_proc->scene.regs[2] = 0;    // set $v0 to 0

    return new_proc->id;
}
