#ifndef _PRINTF_H_
#define _PRINTF_H_

#include <efi.h>
#include <types.h>

void clear_screen();
void putchar(char ch);
void puts(char* str);
void printhex(size_t number);
void printdec(ssize_t number);
void printf(const char* fmt, ...);
void printf_init(EFI_SYSTEM_TABLE* SysTab);

#endif
