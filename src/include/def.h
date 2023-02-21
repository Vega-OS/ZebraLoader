/*
 *  @description: def.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _DEF_H_
#define _DEF_H_

#include <efi.h>

extern EFI_BOOT_SERVICES* BS;
extern EFI_SYSTEM_TABLE* ST;

#define halt()                        \
  printf("-- System halted --\n");    \
  __asm__ __volatile__("cli; hlt")

#endif
