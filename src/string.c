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
