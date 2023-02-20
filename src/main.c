#include <efi.h>
#include <printf.h>

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  (void)ImageHandle;
  printf_init(SystemTable);

  clear_screen();
  printf("-- Welcome to ZebraLoader --\n");
  printf("Hello, wold!\n");

  for (;;);
}
