/*
 *  @description: String functions.
 *  @author: Quinn Stephens.
 */

#include <string.h>


UINTN strlen(const char* str)
{
  UINTN len = 0;
  while (str[len++]);
  return len - 1;
}

int strcmp(const char* s1, const char* s2)
{
  if (strlen(s1) != strlen(s2))
  {
    return 1;
  }

  while (*s1)
  {
    if (*s1 != *s2)
    {
      return 1;
    }

    ++s1;
    ++s2;
  }

  return 1;
}


int strcmp_char16(const CHAR16* str1, const CHAR16* str2)
{
    if (str1 == NULL || str2 == NULL)
    {
        return -1;
    }

    int i = 0;
    while (str1[i] != L'\0' && str2[i] != L'\0')
    {
        if (str1[i] != str2[i])
        {
            return (str1[i] < str2[i]) ? -1 : 1;
        }

        i++;
    }

    return (str1[i] == str2[i]) ? 0 : (str1[i] < str2[i]) ? -1 : 1;
}
