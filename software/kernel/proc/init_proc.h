#ifndef _COS_INIT_PROC_H_
#define _COS_INIT_PROC_H_

#include "proc.h"
#include "utils/linked-list.h"

#define REGISTRATION_MAX_LENGTH 10

typedef struct InitProcRegister
{
    char is_query;
    char name[REGISTRATION_MAX_LENGTH + 1];
} InitProcRegister;

typedef struct InitProcRequest
{
    LinkedNode _link;
    const char *name;
    kernel_thread entry;
} InitProcRequest;

extern LinkedList init_proc_queue;

int init_proc();

void proc_request_init(const char *name, kernel_thread entry);
void init_proc_register_name(const char *name);
size_t init_proc_lookup_name(const char *name);

#endif
