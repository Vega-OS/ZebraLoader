/*
 *  @description: Commonly used definitions.
 *  @author: Ian Marco Moffett.
 */

#ifndef _DEF_H_
#define _DEF_H_

#include <efi.h>
#include <efilib.h>

#define __asm       __asm__ __volatile__
#define __packed    __attribute__((packed))
#define SAFE_DIV(num, den) ((den) == 0 ? 0 : (num) / (den))

#define halt()                            \
  Print(L"-- System Halted --\n");        \
  __asm("cli; hlt")

#endif
