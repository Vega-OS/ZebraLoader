/*
 *  @description: string.h
 *  @author: Quinn Stephens
 */

#ifndef _STRING_H_
#define _STRING_H_

#include <def.h>

UINTN strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strcmp_char16(const CHAR16* str1, const CHAR16* str2);

#endif
