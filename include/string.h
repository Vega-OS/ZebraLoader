/*
 *  @description: string.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _STRING_H_
#define _STRING_H_

#include <def.h>

UINTN strlen(const char *str);
void memzero(void* mem, UINTN bytes);

#endif
