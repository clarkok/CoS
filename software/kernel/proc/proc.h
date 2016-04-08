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

typedef struct ProcScene
{
    size_t pc;
    size_t regs[31];
    size_t lo;
    size_t hi;
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

    /**
     * Process ID
     */
    size_t id;

    /**
     * Parent process ID
     */
    size_t parent;

    /**
     * current state
     */
    int state;

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

    MemoryManagement mm;
} Process;

extern Process *current_process;
extern SBTree proc_tree;

#define foreach_proc(proc)  sb_foreach(&proc_tree, proc)

void proc_init();
void proc_schedule();

int proc_do_fork();
int proc_do_get_pid();
void proc_do_set_pname();
size_t proc_do_get_proc_nr();

static inline ProcScene *
proc_current_scene(Process *proc)
{ return (ProcScene*)proc->current_scene; }

#endif
