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
