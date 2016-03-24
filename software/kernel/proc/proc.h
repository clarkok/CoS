#ifndef _COS_PROC_H_
#define _COS_PROC_H_

#include "core/cos.h"

#include "utils/linked-list.h"
#include "utils/sb-tree.h"

#include "mm/mm.h"

typedef struct ProcScene
{
    size_t pc;
    size_t regs[31];
    size_t lo;
    size_t hi;
} ProcScene;

typedef struct Process
{
    LinkedNode _link;
    SBNode _node;

    size_t id;
    char name[11];

    size_t privilege;
    size_t ticks;

    MemoryManagement mm;
    volatile ProcScene state;
} Process;

extern volatile ProcScene *proc_current_scene;

#endif
