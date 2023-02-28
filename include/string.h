/*
 *  @description: string.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _STRING_H_
#define _STRING_H_

#include <def.h>

UINTN strlen(const char *str);
void memzero(void* mem, UINTN bytes);
void _memset(void* mem, UINT8 byte, UINTN bytes);
void _memcpy(void* to, void* from, UINTN bytes);

#endif
