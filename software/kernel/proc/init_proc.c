#include <string.h>
#include <stdlib.h>

#include "init_proc.h" 

#include "lib/sysapi.h"
#include "lib/kernelio.h"

#include "proc/message.h"
#include "utils/sb-tree.h"

LinkedList init_proc_queue;
SBTree init_registration;

typedef struct RegistrationNode
{
    SBNode _node;
    size_t pid;
    char name[REGISTRATION_MAX_LENGTH + 1];
} RegistrationNode;

static void
_reg_insert(SBTree *tree, RegistrationNode *reg)
{
    SBNode **ptr = &sb_root(tree),
           *parent = NULL;

    while (*ptr) {
        parent = *ptr;
        int compare_result = strcmp(
            sb_get(*ptr, RegistrationNode, _node)->name,
            reg->name
        );

        if (compare_result == 0) {
            sb_get(*ptr, RegistrationNode, _node)->pid = reg->pid;
            free(reg);
            return;
        }
        if (compare_result < 0) {
            ptr = &(*ptr)->left;
        }
        else {
            ptr = &(*ptr)->right;
        }
    }
    sb_link(&reg->_node, parent, ptr, tree);
}

static RegistrationNode *
_reg_find(SBTree *tree, const char *name)
{
    SBNode *node = sb_root(tree);

    while (node) {
        RegistrationNode *reg = sb_get(node, RegistrationNode, _node);
        int compare_result = strcmp(reg->name, name);
        if (compare_result == 0) {
            return reg;
        }
        if (compare_result < 0) {
            node = node->left;
        }
        else {
            node = node->right;
        }
    }
    return NULL;
}

int
init_proc(void)
{
    k_printf(
            "Hello to init proc!\n"
            "    current pid is: 0x%X\n",
            k_get_pid()
        );

    if (!k_fork()) {
        k_set_pname("sys_idle");
        k_request_lowest();
        for (;;) ;
    }

    list_for_each(&init_proc_queue, node) {
        InitProcRequest *req = list_get(node, InitProcRequest, _link);
        if (!k_fork()) {
            k_set_pname(req->name);
            k_exit(req->entry());
        }
    }

    while (true) {
        size_t sender = 0;
        size_t msg_size = k_msg_wait_for(0);
        char *buffer = malloc(msg_size);
        k_msg_recv_for(0, &sender, buffer);
        if (sender == SIGNAL_SRC) {
            SignalType type = *(SignalType*)(buffer);
            if (type == ST_CHILD_TERM) {
                SignalChildTerm *signal = (SignalChildTerm*)(buffer);
                int retval;
                k_collect(signal->child_pid, &retval);
            }
        }
        else {
            InitProcRegister *request = (InitProcRegister*)(buffer);
            if (request->is_query) {
                RegistrationNode *reg = _reg_find(&init_registration, request->name);
                size_t ret = reg ? reg->pid : 0;
                k_msg_send(sender, sizeof(size_t), &ret);
            }
            else {
                RegistrationNode *reg = (RegistrationNode*)malloc(sizeof(RegistrationNode));
                reg->pid = sender;
                strcpy(reg->name, request->name);
                _reg_insert(&init_registration, reg);
            }
        }
        free(buffer);
    }
}

void
proc_request_init(const char *name, kernel_thread entry)
{
    InitProcRequest *req = kmalloc(sizeof(InitProcRequest));
    req->name = name;
    req->entry = entry;

    list_append(&init_proc_queue, &req->_link);
}

void
init_proc_register_name(const char *name)
{
    InitProcRegister reg;
    reg.is_query = 0;
    strncpy(reg.name, name, REGISTRATION_MAX_LENGTH);
    k_msg_send(1, sizeof(reg), &reg);
}

size_t
init_proc_lookup_name(const char *name)
{
    InitProcRegister reg;
    reg.is_query = 1;
    strncpy(reg.name, name, REGISTRATION_MAX_LENGTH);
    k_msg_send(1, sizeof(reg), &reg);

    size_t msg_size = k_msg_wait_for(1);
    assert(msg_size == sizeof(size_t));
    size_t ret;
    k_msg_recv_for(1, NULL, (char*)&ret);
    return ret;
}
