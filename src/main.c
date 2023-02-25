/*
 *  @description: Main entrypoint.
 *  @author: Ian Marco Moffett.
 */

#include <def.h>
#include <string.h>
#include <menu.h>
#include <dev/gop.h>
#include <dev/disk.h>
#include <mm/pmm.h>

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *st)
{
  InitializeLib(image_handle, st);
  uefi_call_wrapper(ST->ConOut->Reset, 2, ST->ConOut, 0);
  uefi_call_wrapper(BS->SetWatchdogTimer,    // Disable the watchdog timer
                    4,
                    0,
                    0,
                    0,
                    NULL
  );

  // Check if the boot entryname is of good length.
  if (strlen(BOOT_ENTRYNAME) > 22)
  {
    Print(L"Boot entryname length > 22\n");
    halt();
  }

  gop_init();  // Verify GOP and set native mode
  pmm_init();

  disk_init(image_handle);
  menu_start(); 
  return EFI_SUCCESS;
}
