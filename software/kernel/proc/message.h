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

#endif
