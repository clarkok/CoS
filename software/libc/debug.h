#ifndef _C_LIB_DEBUG_H_
#define _C_LIB_DEBUG_H_

#define static_assert(cond, msg)     \
    typedef char static_assertion_##msg[(cond) ? 1 : -1]

#endif // _C_LIB_DEBUG_H_
