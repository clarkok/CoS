#include <string.h>

#include "message.h"
#include "proc.h"

static inline void
_proc_msg_dispatch(Process *dst_proc, Message *msg)
{
    list_append(&dst_proc->messages, &msg->_link);
    if (
        (dst_proc->state == PS_WAITING) &&
        (!dst_proc->waiting_for || dst_proc->waiting_for == msg->src)
    ) {
        proc_unblock(dst_proc->id, msg->length);
    }
}

Message *
proc_new_message(size_t src, size_t dst, size_t length)
{
    Message *ret = kmalloc(sizeof(Message) + length);
    ret->src = src;
    ret->dst = dst;
    ret->length = length;

    return ret;
}

int
proc_send_message(Message *msg)
{
    Process *dst_proc = proc_get_by_id(msg->dst);
    if (!dst_proc) {
        return 0;
    }
    _proc_msg_dispatch(dst_proc, msg);
    return 1;
}

int
proc_msg_do_send(size_t dst, size_t length, const void *content)
{
    Message *new_msg = proc_new_message(current_process->id, dst, length);
    if (!proc_send_message(new_msg)) {
        kfree(new_msg);
        return 0;
    }
    memcpy(new_msg->content, content, length);
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
            Message *msg = list_get(list_head(&current_process->messages), Message, _link);
            return msg->length;
        }
    }
    proc_block(current_process->id, src);
    return 0;
}

int
proc_msg_do_recv_for(size_t src, size_t *actual_src, char *buffer)
{
    if (src) {
        list_for_each(&current_process->messages, node) {
            Message *msg = list_get(node, Message, _link);
            if (msg->src == src) {
                if (actual_src) {
                    *actual_src = msg->src;
                }
                memcpy(buffer, msg->content, msg->length);
                list_unlink(node);
                kfree(msg);
                return 1;
            }
        }
    }
    else {
        if (list_size(&current_process->messages)) {
            LinkedNode *node = list_head(&current_process->messages);
            Message *msg = list_get(node, Message, _link);
            if (actual_src) {
                *actual_src = msg->src;
            }
            memcpy(buffer, msg->content, msg->length);
            list_unlink(node);
            kfree(msg);
            return 1;
        }
    }
    return 0;
}

size_t
proc_msg_do_get_msg_nr()
{ return list_size(&current_process->messages); }

int
proc_msg_signal(size_t dst, size_t length, const void *content)
{
    Message *new_sig = proc_new_message(SIGNAL_SRC, dst, length);
    if (!proc_send_message(new_sig)) {
        kfree(new_sig);
        return 0;
    }
    memcpy(new_sig->content, content, length);
    return 1;
}

int
proc_msg_interrupt(size_t dst, size_t length, const void *content)
{
    Message *new_sig = proc_new_message(INTERRUPT_SRC, dst, length);
    if (!proc_send_message(new_sig)) {
        kfree(new_sig);
        return 0;
    }
    memcpy(new_sig->content, content, length);
    return 1;
}
