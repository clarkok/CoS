#include "string.h"

static inline void
_memcpy_aligned(void *dst, const void *src, size_t num)
{
    uint32_t *dst_i = (uint32_t*)dst,
             *src_i = (uint32_t*)src;
    size_t count = ((num >> 2) + 7) >> 3;
    if (!num) return;
    switch ((num >> 2) & 0x7) {
    case 0: do { *dst_i++ = *src_i++;
    case 7:      *dst_i++ = *src_i++;
    case 6:      *dst_i++ = *src_i++;
    case 5:      *dst_i++ = *src_i++;
    case 4:      *dst_i++ = *src_i++;
    case 3:      *dst_i++ = *src_i++;
    case 2:      *dst_i++ = *src_i++;
    case 1:      *dst_i++ = *src_i++;
            } while(--count > 0);
    }
}

static inline void
_memcpy_unaligned(void *dst, const void *src, size_t num)
{
    uint8_t *dst_i = (uint8_t*)dst,
            *src_i = (uint8_t*)src;
    size_t count = (num + 7) >> 3;
    if (!num) return;
    switch (num & 0x7) {
    case 0: do { *dst_i++ = *src_i++;
    case 7:      *dst_i++ = *src_i++;
    case 6:      *dst_i++ = *src_i++;
    case 5:      *dst_i++ = *src_i++;
    case 4:      *dst_i++ = *src_i++;
    case 3:      *dst_i++ = *src_i++;
    case 2:      *dst_i++ = *src_i++;
    case 1:      *dst_i++ = *src_i++;
            } while(--count > 0);
    }
}

void *
memcpy(void *dst, const void *src, size_t num)
{
    size_t padding;
    void *ret = dst;

    if (((size_t)dst & 0x3) == ((size_t)src & 0x3)) {
        padding = 4 - ((size_t)dst & 0x3);

        _memcpy_unaligned(dst, src, padding);
        _memcpy_aligned(
                dst += padding,
                src += padding,
                num -= padding
            );
        padding = num & 0x3;
        num -= padding;
        _memcpy_unaligned(
                dst += num,
                src += num,
                padding
            );
    }
    else {
        _memcpy_unaligned(dst, src, num);
    }

    return ret;
}

void
_memcpy_back_aligned(void *dst, const void *src, size_t num) __attribute__((always_inline))
{
    uint32_t *dst_i = (uint32_t*)dst + (num >> 2),
             *src_i = (uint32_t*)src + (num >> 2);
    size_t count = ((num >> 2) + 7) >> 3;
    if (!num) return;
    switch ((num >> 2) & 0x7) {
    case 0: do { *--dst_i = *--src_i;
    case 7:      *--dst_i = *--src_i;
    case 6:      *--dst_i = *--src_i;
    case 5:      *--dst_i = *--src_i;
    case 4:      *--dst_i = *--src_i;
    case 3:      *--dst_i = *--src_i;
    case 2:      *--dst_i = *--src_i;
    case 1:      *--dst_i = *--src_i;
            } while(--count > 0);
    }
}

void
_memcpy_back_unaligned(void *dst, const void *src, size_t num)  __attribute__((always_inline))
{
    uint8_t *dst_i = (uint8_t*)dst + num,
            *src_i = (uint8_t*)src + num;
    size_t count = (num + 7) >> 3;
    if (!num) return;
    switch (num & 0x7) {
    case 0: do { *--dst_i = *--src_i;
    case 7:      *--dst_i = *--src_i;
    case 6:      *--dst_i = *--src_i;
    case 5:      *--dst_i = *--src_i;
    case 4:      *--dst_i = *--src_i;
    case 3:      *--dst_i = *--src_i;
    case 2:      *--dst_i = *--src_i;
    case 1:      *--dst_i = *--src_i;
            } while(--count > 0);
    }
}

void *
memcpy_back(void *dst, const void *src, size_t num)
{
    size_t padding;
    void *ret = dst;

    if (((size_t)dst & 0x3) == ((size_t)src & 0x3)) {
        padding = ((size_t)dst + num) & 0x3;
        _memcpy_back_unaligned(
                dst + num - padding,
                src + num - padding,
                padding
            );
        padding = (size_t)dst & 0x3;
        _memcpy_back_aligned(
                dst + padding,
                src + padding,
                num - padding
            );
        _memcpy_back_unaligned(dst, src, padding);
    }
    else {
        _memcpy_back_unaligned(dst, src, num);
    }

    return ret;
}

char *
strcpy(char *dst, const char *src)
{
    char *ret = dst;
    while ((*dst++ = *src++));
    return ret;
}

char *
strncpy(char *dst, const char *src, size_t num)
{
    char *ret = dst;
    while (num--) { if (!(*dst++ = *src++)) return ret; }
    *dst = 0;
    return ret;
}

char *
strcat(char *dst, const char *src)
{
    strcpy(dst + strlen(dst), src);
    return dst;
}

char *
strncat(char *dst, const char *src, size_t num)
{
    strncpy(dst + strlen(dst), src, num);
    return dst;
}

int
memcmp(const void *lhs, const void *rhs, size_t num)
{
    const uint8_t *l = (const uint8_t *)lhs,
                  *r = (const uint8_t *)rhs;

    while (num--) {
        if (*l != *r) { return *l - *r; }
        ++l; ++r;
    }

    return 0;
}

int
strcmp(const char *lhs, const char *rhs)
{
    while (*lhs && *rhs) {
        if (*lhs != *rhs) { return *lhs - *rhs; }
        ++lhs; ++rhs;
    }
    return *lhs - *rhs;
}

int
strncmp(const char *lhs, const char *rhs, size_t num)
{
    const char *l = (const char *)lhs,
               *r = (const char *)rhs;

    while (num--) {
        if (*l != *r) { return *l - *r; }
        ++l; ++r;
    }

    return 0;
}

void *
memchr(const void *ptr, int value, size_t num)
{
    while (num--) {
        if (*(uint8_t*)ptr == (uint8_t)value) return (void*)ptr;
        ++ptr;
    }
    return NULL;
}

char *
strchr(const char *str, int value)
{
    while (*str) {
        if (*str == value) return (char*)str;
        ++str;
    }
    return NULL;
}

size_t
strcspn(const char *dst, const char *src)
{
    const char *p = dst;
    while (*p) {
        if (strchr(src, *p)) return p - dst;
        ++p;
    }
    return p - dst;
}

char *
strpbrk(const char *dst, const char *breakset)
{
    while (*dst) {
        if (strchr(breakset, *dst)) return (char*)dst;
        ++dst;
    }
    return NULL;
}

char *
strrchr(const char *str, int value)
{
    const char *p = str + (strlen(str) - 1);

    while (p >= str) {
        if (*p == value) { return (char*)p; }
        --p;
    }
    return NULL;
}

size_t
strspn(const char *dst, const char *src)
{
    const char *p = dst;
    while (*p) {
        if (!strchr(src, *p)) return p - dst;
        ++p;
    }
    return p - dst;
}

static inline void
_memset_aligned(void *dst, int ch, size_t num)
{
    uint32_t *dst_i = (uint32_t*)dst;
    uint32_t value = ((uint8_t)ch << 24) | 
                     ((uint8_t)ch << 16) |
                     ((uint8_t)ch << 8)  |
                     ((uint8_t)ch);
    size_t count = ((num >> 2) + 7) >> 3;
    if (!num) return;
    switch ((num >> 2) & 0x7) {
    case 0: do { *dst_i++ = value;
    case 7:      *dst_i++ = value;
    case 6:      *dst_i++ = value;
    case 5:      *dst_i++ = value;
    case 4:      *dst_i++ = value;
    case 3:      *dst_i++ = value;
    case 2:      *dst_i++ = value;
    case 1:      *dst_i++ = value;
            } while(--count > 0);
    }
}

static inline void
_memset_unaligned(void *dst, int ch, size_t num)
{
    uint8_t *dst_i = dst;

    while (num--) {
        *dst_i++ = (uint8_t)ch;
    }
}

void *
memset(void *dst, int ch, size_t num)
{
    void *ret = dst;
    size_t padding = 4 - ((size_t)dst & 0x3);

    if (!num) return dst;
    if (num <= padding) {
        _memset_unaligned(dst, ch, num);
        return dst;
    }

    _memset_unaligned(dst, ch, padding);
    _memset_aligned(dst += padding, ch, num -= padding);
    padding = num & 0x3;
    num -= num - padding;
    _memset_unaligned(dst += num, ch, padding);

    return ret;
}

size_t
strlen(const char *str)
{
    size_t length = 0;

    while (*str++) {
        ++length;
    }

    return length;
}
