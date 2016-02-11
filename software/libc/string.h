#ifndef _C_LIB_STRING_H_
#define _C_LIB_STRING_H_

#include "debug.h"
#include "stddef.h"

void *memcpy(void *dst, const void *src, size_t num);
// void *memmove(void *dst, const void *src, size_t num);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t num);

char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t num);

int memcmp(const void *lhs, const void *rhs, size_t num);
int strcmp(const char *lhs, const char *rhs);
int strncmp(const char *lhs, const char *rhs, size_t num);

void *memchr(const void *ptr, int value, size_t num);
char *strchr(const char *str, int value);
size_t strcspn(const char *dst, const char *src);
char *strpbrk(const char *dst, const char *breakset);
char *strrchr(const char *str, int value);
size_t strspn(const char *dst, const char *src);
// char *strstr(const char *str, const char *substr);
// char *strtok(char *str, const char *delim);

void *memset(void *dst, int ch, size_t count);
size_t strlen(const char *str);

#endif // _C_LIB_STRING_H_
