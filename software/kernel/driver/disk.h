#ifndef _COS_DISK_H_
#define _COS_DISK_H_

#include <assert.h>

#include "core/kernel.h"
#include "core/cos.h"

enum DiskResponseStatusCode
{
    DS_OK = 0,
    DS_INVALID_REQ_SIZE,
};

typedef struct DiskRequestMessage
{
    int request_id;
    unsigned int is_read : 1;
    unsigned int block_id: 31;
    char content[0];
} DiskRequestMessage;

typedef struct DiskResponseMessage
{
    int request_id;
    int status_code;
    char content[0];
} DiskResponseMessage;

static_assert(sizeof(DiskRequestMessage) == 8, "sizeof DiskRequestMessage should be 8");
static_assert(sizeof(DiskResponseMessage) == 8, "sizeof DiskResponseMessage should be 8");

void disk_init();

int disk_read_async(int request_id, size_t block_id);
int disk_write_async(int request_id, size_t block_id, const char *buffer);
int disk_get_pid();

#endif
