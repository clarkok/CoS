#ifndef _COS_PROC_H_
#define _COS_PROC_H_

#include "core/cos.h"

#include "utils/linked-list.h"
#include "utils/sb-tree.h"

#include "mm/mm.h"

enum ProcState
{
    PS_READY,
    PS_RUNNING,
    PS_WAITING,
    PS_ZOMBIE
};

enum ProcPrivilege
{
    PRIV_LOWEST = 0,
    PRIV_NORMAL = 16,
    PRIV_REALTIME = 1024
};

#define SIGNAL_SRC      (0xFFFFFFFFu)
#define INTERRUPT_SRC   (0xFFFFFFFEu)

typedef struct ProcScene
{
    size_t pc;
    size_t regs[31];
    size_t lo;
    size_t hi;
    struct ProcScene *last_scene;
} ProcScene;

#define PROC_NAME_LENGTH 11

typedef struct Process
{
    // for assembly
    char *kernel_stack_top;
    char *kernel_stack;
    volatile ProcScene *current_scene;

    LinkedNode _link;
    SBNode _node;
    LinkedNode _child_link;

    /**
     * Process ID
     */
    size_t id;

    /**
     * current state
     */
    int state;

    /**
     * process return value
     */
    int retval;

    /**
     * Process name
     */
    char name[PROC_NAME_LENGTH + 1];

    /**
     * Process privilege
     * 0    lowest privilege, would only be running when there is no any process able to run
     * 16   normal privilege
     * 1024 realtime privilege
     */
    size_t priv_base;

    /**
     * For dynamic privilege
     */
    int priv_offset;

    /**
     * ticks remains
     */
    size_t ticks;

    /**
     * Pointer to Parent
     */
    struct Process *parent;

    /**
     * Children list
     */
    LinkedList children;

    LinkedList messages;
    size_t waiting_for;

    MemoryManagement mm;
} Process;

extern Process *current_process;
extern SBTree proc_tree;
extern int proc_request_schedule;

typedef int (*kernel_thread)(void);

#define foreach_proc(proc)  sb_foreach(&proc_tree, proc)

void proc_init();
void proc_schedule();

Process *proc_get_by_id(size_t id);

void proc_block(size_t pid, size_t waiting_for);
void proc_unblock(size_t pid, size_t retval);
void proc_zombie(Process *proc, int retval);
void proc_request_init(const char *name, kernel_thread entry);

int proc_do_fork();
int proc_do_get_pid();
void proc_do_set_pname();
size_t proc_do_get_proc_nr();
void proc_do_exit(int retval);
int proc_do_collect(size_t pid, int *retval);
void proc_do_giveup();
void proc_do_request_lowest();

static inline ProcScene *
proc_current_scene(Process *proc)
{ return (ProcScene*)proc->current_scene; }

#endif
