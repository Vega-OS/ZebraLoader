/*
 *  @description: String library.
 *  @author: Quinn Stephens
 */

#include <string.h>

size_t strlen(const char* str)
{
  size_t len = 0;
  while (str[len++]);
  return len - 1;
}


void* memset(void* s, int c, size_t n)
{
  for (size_t i = 0; i < n; ++i)
  {
    ((char*)s)[i] = c;
  }

  return s;
}

void memcpy(void* dst, const void* src, size_t n)
{
  for (size_t i = 0; i < n; ++i)
  {
    ((char*)dst)[i] = ((char*)src)[i];
  }
}

int memcmp(const void* s1, const void* s2, size_t n)
{
  if (n != 0) {
    const unsigned char *p1 = s1, *p2 = s2;

    do {
      if (*p1++ != *p2++)
        return (*--p1 - *--p2);
    } while (--n != 0);
  }
  return (0);
}

int strcmp(const char* s1, const char* s2)
{
  while (*s1 == *s2++)
    if (*s1++ == 0)
      return (0);
  return (*(unsigned char*)s1 - *(unsigned char*)--s2);
}
