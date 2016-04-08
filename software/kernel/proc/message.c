#include <string.h>

#include "message.h"
#include "proc.h"

int
proc_msg_do_send(size_t dst, size_t length, const void *content)
{
    Process *dst_proc = proc_get_by_id(dst);
    if (!dst_proc) {
        return 0;
    }

    Message *new_msg = malloc(sizeof(Message) + length);
    new_msg->src = current_process->id;
    new_msg->dst = dst;
    new_msg->length = length;
    memcpy(new_msg->content, content, length);

    list_append(&dst_proc->messages, &new_msg->_link);

    if (
        (dst_proc->state == PS_WAITING) &&
        (!dst_proc->waiting_for || dst_proc->waiting_for == current_process->id)
    ) {
        proc_unblock(dst_proc->id, length);
    }

    return 1;
}

size_t
proc_msg_do_wait_for(size_t src)
{
    if (src) {
        list_for_each(&current_process->messages, node) {
            Message *msg = list_get(node, Message, _link);
            if (msg->src == src) {
                return msg->length;
            }
        }
    }
    else {
        if (list_size(&current_process->messages)) {
            return list_get(list_head(&current_process->messages), Message, _link)->length;
        }
    }
    proc_block(current_process->id, src);
    return 0;
}

int
proc_msg_do_recv_for(size_t src, char *buffer)
{
    if (src) {
        list_for_each(&current_process->messages, node) {
            Message *msg = list_get(node, Message, _link);
            if (msg->src == src) {
                memcpy(buffer, msg->content, msg->length);
                list_unlink(node);
                free(msg);
                return 1;
            }
        }
    }
    else {
        if (list_size(&current_process->messages)) {
            LinkedNode *node = list_head(&current_process->messages);
            Message *msg = list_get(node, Message, _link);
            memcpy(buffer, msg->content, msg->length);
            list_unlink(node);
            free(msg);
            return 1;
        }
    }
    return 0;
}

size_t
proc_msg_do_get_msg_nr()
{ return list_size(&current_process->messages); }
