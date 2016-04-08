#include <string.h>

#include "mm/mm.h"
#include "core/kernel.h"

#include "proc.h"

#define proc_kernel_thread(func)    (((size_t)(func)) + 1)

Process *current_process;
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

void init_proc();

static Process *
process_new(const char *p_name, Process *parent)
{
    Process *new_proc = malloc(sizeof(Process));
    new_proc->kernel_stack = malloc(PAGE_SIZE);
    new_proc->kernel_stack_top = new_proc->kernel_stack + PAGE_SIZE - sizeof(ProcScene);
    new_proc->current_scene = (volatile ProcScene*)new_proc->kernel_stack_top;
    new_proc->current_scene->last_scene = 0;
    new_proc->id = ++_proc_id;
    new_proc->state = PS_READY;
    strncpy(new_proc->name, p_name, PROC_NAME_LENGTH);
    new_proc->priv_base = PRIV_NORMAL;
    new_proc->priv_offset = 0;
    new_proc->ticks = 0;
    new_proc->parent = parent;
    list_init(&new_proc->children);
    list_init(&new_proc->messages);
    new_proc->waiting_for = 0;

    if (parent) {
        list_append(&parent->children, &new_proc->_child_link);
        mm_duplicate(&new_proc->mm, &parent->mm);
        memcpy(proc_current_scene(new_proc), proc_current_scene(parent), sizeof(ProcScene) - 4);    // for last_scene
        proc_current_scene(new_proc)->regs[1] = 0;
    }
    else {
        mm_init_proc(&new_proc->mm);
        memset(proc_current_scene(new_proc), 0, sizeof(ProcScene));
    }

    _proc_insert(&proc_tree, new_proc);
    _proc_insert_into_queues(new_proc);

    return new_proc;
}

static Process *
_proc_construct_init()
{
    Process *new_proc = process_new("init", NULL);

    new_proc->current_scene->pc = proc_kernel_thread(init_proc);

    return new_proc;
}

void
proc_init()
{
    dbg_uart_str("Proc init\n");

    _proc_id = 0;

    sb_init(&proc_tree);
    list_init(&proc_zero_queue);
    list_init(&proc_normal_queue);
    list_init(&proc_normal_noticks_queue);
    list_init(&proc_realtime_queue);

    Process *init_proc = _proc_construct_init();

    proc_schedule();    // update states

    ProcScene *scene = proc_current_scene(init_proc);
    scene->regs[28] = (size_t)mm_do_mmap_empty(PAGE_SIZE, USER_SPACE_SIZE - PAGE_SIZE) + PAGE_SIZE;
}

void
proc_schedule()
{
    dbg_uart_str("schedule ");

    Process *proc_to_run = NULL;
    if (list_size(&proc_realtime_queue)) {
        proc_to_run = list_get(list_head(&proc_realtime_queue), Process, _link);
    }
    else if (list_size(&proc_normal_queue)) {
        if (list_from_node(&current_process->_link) == &proc_normal_queue &&
            list_next(&current_process->_link)) {
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
        if (list_from_node(&current_process->_link) == &proc_zero_queue &&
            list_next(&current_process->_link)) {
            proc_to_run = list_get(list_next(&current_process->_link), Process, _link);
        }
        else {
            proc_to_run = list_get(list_head(&proc_zero_queue), Process, _link);
        }
    }

    assert(proc_to_run);

    mm_set_page_table(&proc_to_run->mm);
    current_process = proc_to_run;

    dbg_uart_str("to ");
    dbg_uart_str(proc_to_run->name);
    dbg_uart_str(" ");
    dbg_uart_hex(proc_to_run->id);
}

int
proc_do_fork()
{ return process_new(current_process->name, current_process)->id; }

int
proc_do_get_pid()
{ return current_process->id; }

void
proc_do_set_pname(const char *name)
{ strncpy(current_process->name, name, PROC_NAME_LENGTH); }

size_t
proc_do_get_proc_nr()
{ return sb_size(&proc_tree); }
