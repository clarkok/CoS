#include <string.h>
#include <stdlib.h>

#include "disk.h"

#include "lib/sysapi.h"
#include "lib/kernelio.h"

#include "proc/proc.h"
#include "proc/message.h"
#include "utils/linked-list.h"

typedef struct LinkedDiskRequest
{
    LinkedNode _link;
    size_t sender;
    DiskRequestMessage msg;
    char buffer[0];
} LinkedDiskRequest;

typedef struct DiskResponseMessageInStack
{
    DiskResponseMessage msg;
    char content[DISK_BLOCK_SIZE];
} DiskResponseMessageInStack;

typedef struct DiskRequestMessageInStack
{
    DiskRequestMessage msg;
    char content[DISK_BLOCK_SIZE];
} DiskRequestMessageInStack;

static_assert(
        sizeof(DiskResponseMessageInStack) == DISK_BLOCK_SIZE + sizeof(DiskResponseMessage),
        "DiskResponseMessageInStack should layout properly"
);

volatile size_t disk_proc_pid = 0;

static void
disk_reply(size_t sender, int request_id, int status_code)
{
    DiskResponseMessage res;
    res.request_id = request_id;
    res.status_code = status_code;

    k_msg_send(sender, sizeof(res), &res);
}

static void
disk_reply_data(size_t sender, int request_id, int status_code, const char *data)
{
    DiskResponseMessageInStack res;
    res.msg.request_id = request_id;
    res.msg.status_code = status_code;
    memcpy(res.content, data, DISK_BLOCK_SIZE);

    k_msg_send(sender, sizeof(res), &res);
}

#define disk_flash_status(ready, read, write, block)    \
    ((!!(ready) << 31) | (!!(read) << 30) | (!!(write) << 29) | ((block) & ((1 << 29) - 1)))

#define disk_get_ready_from_status(status)              \
    ((status) & (1 << 31))

#define disk_get_read_from_status(status)               \
    ((status) & (1 << 30))

#define disk_get_write_from_status(status)              \
    ((status) & (1 << 29))

#define disk_get_block_from_status(status)              \
    ((status) & ((1 << 29) - 1))

static volatile char   * const FLASH_DATA = (volatile char   *)(0xFFFFFC00u);
static volatile size_t * const FLASH_CTRL = (volatile size_t *)(0xFFFFFE00u);

static void
disk_perform_operation(LinkedDiskRequest *request)
{
    if (!request->msg.is_read) {
        memcpy((void*)FLASH_DATA, request->buffer, DISK_BLOCK_SIZE);
    }
    *FLASH_CTRL = disk_flash_status(
            0,
            request->msg.is_read,
            !request->msg.is_read,
            request->msg.block_id
    );
}

static void
disk_insert_request_into_queue(LinkedList *queue, LinkedDiskRequest *request)
{
    LinkedNode *ptr = list_head(queue);
    while (ptr) {
        if (list_get(ptr, LinkedDiskRequest, _link)->msg.block_id > request->msg.block_id) {
            list_before(ptr, &request->_link);
            return;
        }
    }
    list_append(queue, &request->_link);
}

static int
disk_proc(void)
{
    LinkedList disk_request_queue;
    LinkedDiskRequest *current_request = NULL;
    int current_forward = 1;

    disk_proc_pid = k_get_pid();
    list_init(&disk_request_queue);

    while (true) {
        size_t msg_size = k_msg_wait_for(0);
        LinkedDiskRequest *req = malloc(sizeof(LinkedDiskRequest) + DISK_BLOCK_SIZE);
        assert(req);
        char *buffer = (char*)&req->msg;

        k_msg_recv_for(0, &req->sender, buffer);

        if (req->sender == INTERRUPT_SRC) {
            assert(msg_size == sizeof(size_t));
            size_t *current_status = (size_t*)buffer;

            assert(disk_get_block_from_status(*current_status) == current_request->msg.block_id);

            if (current_request->msg.is_read) {
                disk_reply_data(
                        current_request->sender,
                        current_request->msg.request_id,
                        DS_OK,
                        (const char*)FLASH_DATA
                    );
            }
            else {
                disk_reply(
                        current_request->sender,
                        current_request->msg.request_id,
                        DS_OK
                    );
            }
            free(req);

            LinkedDiskRequest *next_request = NULL;
            if (list_size(&disk_request_queue) != 1) {
                LinkedNode *next_node = current_forward ? list_next(&current_request->_link)
                                                        : list_prev(&current_request->_link);
                if (!next_node) {
                    current_forward = !current_forward;
                    next_node = current_forward ? list_head(&disk_request_queue)
                                                : list_tail(&disk_request_queue);
                }

                next_request = list_get(
                        next_node,
                        LinkedDiskRequest,
                        _link
                    );
            }

            list_unlink(&current_request->_link);
            free(current_request);
            current_request = next_request;

            if (current_request) {
                disk_perform_operation(current_request);
            }
        }
        else if (req->sender == SIGNAL_SRC) {
            assert(0);
        }
        else {
            if (req->msg.is_read) {
                if (msg_size != sizeof(DiskRequestMessage)) {
                    disk_reply(req->sender, req->msg.request_id, DS_INVALID_REQ_SIZE);
                    free(req);
                    continue;
                }
            }
            else {
                if (msg_size != sizeof(DiskRequestMessageInStack)) {
                    disk_reply(req->sender, req->msg.request_id, DS_INVALID_REQ_SIZE);
                    free(req);
                    continue;
                }
            }

            disk_insert_request_into_queue(&disk_request_queue, req);

            if (!current_request) {
                current_request = list_get(list_head(&disk_request_queue), LinkedDiskRequest, _link);
                disk_perform_operation(current_request);
            }
        }
    }

    return 0;
}

void
disk_handler()
{
    size_t current_status = *FLASH_CTRL;
    *FLASH_CTRL = 0;
    proc_msg_interrupt(disk_proc_pid, sizeof(size_t), (const char *)&current_status);
}

void
disk_init()
{
    kprintf("Disk init\n");
    proc_request_init("disk_driver", disk_proc);

    INTERRUPT_HANDLER_TABLE[INT_FLASH] = (size_t)(&disk_handler);
    enable_int(INT_FLASH);
}

int
disk_read_async(int request_id, size_t block_id)
{
    DiskRequestMessage req;

    if (!disk_proc_pid) { return 0; }
    req.request_id = request_id;
    req.is_read = 1;
    req.block_id = block_id;

    return k_msg_send(disk_proc_pid, sizeof(req), (char *)&req);
}

int
disk_write_async(int request_id, size_t block_id, const char *buffer)
{
    DiskRequestMessageInStack req;

    if (!disk_proc_pid) { return 0; }
    req.msg.request_id = request_id;
    req.msg.is_read = 0;
    req.msg.block_id = block_id;
    memcpy(req.content, buffer, DISK_BLOCK_SIZE);

    return k_msg_send(disk_proc_pid, sizeof(req), (char *)&req);
}

int
disk_get_pid()
{ return disk_proc_pid; }
