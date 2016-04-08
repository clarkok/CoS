#ifndef _COS_MESSAGE_H_
#define _COS_MESSAGE_H_

#include "utils/linked-list.h"

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
int proc_msg_do_recv_for(size_t src, char *buffer);
size_t proc_msg_do_get_msg_nr();

#endif
