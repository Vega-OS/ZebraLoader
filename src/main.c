#include <def.h>
#include <dev/gop.h>
#include <dev/disk.h>
#include <menu.h>

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* st)
{
  InitializeLib(image_handle, st);

  uefi_call_wrapper(ST->ConOut->Reset, 2, ST->ConOut, 0);
  disk_init(image_handle);

  gop_init();
  menu_init();

  halt();
}
