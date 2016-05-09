#ifndef _COS_INIT_PROC_H_
#define _COS_INIT_PROC_H_

#include "proc.h"
#include "utils/linked-list.h"

typedef struct InitProcRequest
{
    LinkedNode _linked;
    const char *name;
    kernel_thread entry;
} InitProcRequest;

extern LinkedList init_proc_queue;

int init_proc();

#endif
