#include <def.h>
#include <menu.h>
#include <loader.h>
#include <dev/gop.h>
#include <dev/disk.h>
#include <mm/pmm.h>

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* st)
{
  InitializeLib(image_handle, st);
  uefi_call_wrapper(ST->ConOut->Reset, 2, ST->ConOut, 0);
  pmm_init();

  disk_init(image_handle);

  gop_init();
  menu_init();
 
  uefi_call_wrapper(ST->ConOut->Reset, 2, ST->ConOut, 0);
  load_kernel(image_handle);

  __builtin_unreachable();
}
