/*
 *  @description: Main entrypoint.
 *  @author: Ian Marco Moffett.
 */

#include <def.h>
#include <dev/gop.h>

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

  // Verify GOP and set native mode
  gop_init();
  return EFI_SUCCESS;
}
