#include <efi.h>
#include <efilib.h>

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* st)
{
  InitializeLib(image_handle, st);
  Print(L"Hello, World!\n");
  for (;;);
}
