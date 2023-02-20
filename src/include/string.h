#ifndef _STRING_H_
#define _STRING_H_

#include <types.h>

size_t strlen(const char* str);
void* memset(void* s, int c, size_t n);
void memcpy(void* dst, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
int strcmp(const char* s1, const char* s2);

#endif
