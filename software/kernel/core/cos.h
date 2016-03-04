#ifndef _COS_COS_H_
#define _COS_COS_H_

#include <stddef.h>

#define __COS__
#define __COS_NAME__        "COS"
#define __COS_VERSION__     "0.0.1"

#define container_of(ptr, type, mem)                        \
        ({ const typeof(((type*)0)->mem) *__mptr = (ptr);   \
           (type*)((char*)__mptr - offsetof(type, mem)); })  

#endif 
