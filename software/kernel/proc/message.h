#ifndef _COS_MESSAGE_H_
#define _COS_MESSAGE_H_

#include "utils/linked-list.h"

typedef enum SignalType
{
    ST_UNKNOWN = 0,
    ST_TERM,            // request this process to quit
    ST_CHILD_TERM,      // a child from this process terminated

    SIGNAL_TYPE_NR
} SignalType;

typedef struct SignalChildTerm
{
    SignalType type;
    size_t child_pid;
} SignalChildTerm;

typedef struct Message
{
    LinkedNode _link;

    size_t src;
    size_t dst;
    size_t length;
    char content[0];
} Message;

int proc_msg_do_send(size_t dst, size_t length, const void *content);
size_t proc_msg_do_wait_for(size_t src);
int proc_msg_do_recv_for(size_t src, size_t *actual_src, char *buffer);
size_t proc_msg_do_get_msg_nr();

#define SIGNAL_SRC  (0xFFFFFFFFu)

void proc_msg_signal(size_t dst, size_t length, const void *content);

#endif
