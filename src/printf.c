/*
 *  @description: Printf implementation.
 *  @author: Quinn Stephens
 */

#include <efi.h>
#include <string.h>
#include <stdarg.h>

EFI_SYSTEM_TABLE* SystemTable;
static const char hexChars[16] = "0123456789abcdef";
static unsigned char buf_pos = 0;
CHAR16 buf[256];

void printf_flush()
{
  buf[buf_pos] = '\0';
  SystemTable->ConOut->OutputString(SystemTable->ConOut, buf);
  memset(buf, 0, sizeof(buf));
  buf_pos = 0;
}

void clear_screen()
{
  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
}

void putchar(char ch)
{
  /* Stop a bit before the end of the buffer for safety */
  if(buf_pos >= 250)
  {
    printf_flush();
  }

  /* Microsoft likes CRLF endings */
  if(ch == '\n')
  {
    buf[buf_pos++] = '\r';
  }

  buf[buf_pos++] = ch;
}

void puts(char* str)
{
  for (const char* ptr = str; *str; str++)
  {
    putchar(*ptr);
  }
}

void printhex(size_t number)
{
  uint8_t shr_count = 64;
  while(shr_count)
  {
    shr_count -= 4;
    putchar(hexChars[(number >> shr_count) & 0x0f]);
  }
}

void printdec(ssize_t number) 
{
  uint64_t divisor = 10000000000000000000UL;
  char padding = 1;
  if(number < 0)
  {
    putchar('-');
    number = 0 - number;
  }

  for(;;)
  {
    char ch = '0' + (number / divisor);
    number %= divisor;

    if(ch != '0') padding = 0;
    if(!padding || divisor == 1) putchar(ch);

    if(divisor <= 1) break;
    divisor /= 10;
  }
}


static void print_formatted(char formatter, va_list ap)
{
  switch(formatter)
  {
    case 's':
      puts(va_arg(ap, char*));
      break;
    case 'd':
    case 'u':
    case 'l':
      printdec(va_arg(ap, uint64_t));
      break;
    case 'x':
    case 'p':
      putchar('0');
      putchar('x');
      printhex(va_arg(ap, uint64_t));
      break;
    case 'X':
      printhex(va_arg(ap, uint64_t));
      break;
    case 'c':
      putchar(va_arg(ap, int));
      break;
  }
}

void printf(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  for (const char* ptr = fmt; *ptr; ptr++)
  {
    if(*ptr == '%')
    {
      ptr++;
      print_formatted(*ptr, ap);
    } else {
      putchar(*ptr);
    }
  }

  printf_flush();
  va_end(ap);
}

void printf_init(EFI_SYSTEM_TABLE* SysTab)
{
  SystemTable = SysTab;
  memset(buf, 0, sizeof(buf));
}
