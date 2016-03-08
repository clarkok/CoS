#ifndef _C_LIB_STDINT_H_
#define _C_LIB_STDINT_H_

#include <assert.h>

typedef unsigned int    uint32_t;
typedef unsigned short  uint16_t;
typedef unsigned char   uint8_t;

typedef int             int32_t;
typedef short           int16_t;
typedef char            int8_t;

static_assert((sizeof(uint32_t) == 4), "uint32_t should be size of 4");
static_assert((sizeof(uint16_t) == 2), "uint16_t should be size of 2");
static_assert((sizeof(uint8_t) == 1), "uint8_t should be size of 1");

static_assert((sizeof(int32_t) == 4), "int32_t should be size of 4");
static_assert((sizeof(int16_t) == 2), "int16_t should be size of 2");
static_assert((sizeof(int8_t) == 1), "int8_t should be size of 1");

#endif // _C_LIB_STDINT_H_
