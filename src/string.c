/*
 *  @description: String library.
 *  @author: Ian Marco Moffett.
 */

#include <string.h>

UINTN strlen(const char *str)
{
  UINTN len = 0;
  while (str[len++]);
  return len - 1;
}

void memzero(void* mem, UINTN bytes)
{
  for (UINTN i = 0; i < bytes; ++i)
  {
    ((UINT8*)mem)[i] = 0;
  }
}

void _memset(void* mem, UINT8 byte, UINTN bytes)
{
  for (UINTN i = 0; i < bytes; ++i)
  {
    ((UINT8*)mem)[i] = byte;
  }
}


void _memcpy(void* to, void* from, UINTN bytes)
{
  for (UINTN i = 0; i < bytes; ++i)
  {
    ((UINT8*)to)[i] = ((UINT8*)from)[i];
  }
}
