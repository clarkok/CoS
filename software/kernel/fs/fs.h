#ifndef _COS_FS_H_
#define _COS_FS_H_

#include "core/kernel.h"
#include "core/cos.h"

#include "utils/sb-tree.h"

enum VNodeAttr
{
    V_DIR       = 0,
    V_SEEKABLE  = 1,
};

typedef struct FileDescriptor
{
    SBNode _node;

    size_t fs_pid;
    size_t inode;
    size_t pointer;
} FileDescriptor;

void fs_init();

#endif
