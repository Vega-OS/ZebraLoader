/*
 *  @description: Main entrypoint.
 *  @author: Ian Marco Moffett.
 */

#include <def.h>
#include <dev/gop.h>
#include <dev/disk.h>
#include <menu.h>

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *st)
{
  InitializeLib(image_handle, st);
  uefi_call_wrapper(ST->ConOut->Reset, 2, ST->ConOut, 0);
  uefi_call_wrapper(BS->SetWatchdogTimer,    /* Disable the watchdog timer */
                    4,
                    0,
                    0,
                    0,
                    NULL
  );

  gop_init();  // Verify GOP and set native mode

  disk_init(image_handle);
  menu_start(); 
  return EFI_SUCCESS;
}
