/*
 *  @description: Main entrypoint.
 *  @author: Ian Marco Moffett.
 */

#include <def.h>
#include <dev/gop.h>

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* st)
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

  // Press escape to exit from bootloader
  while (1) {
    EFI_INPUT_KEY key;
    uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &key);

    if ((UINTN)key.ScanCode == SCAN_ESC)
    {
      break;
    }
  }

  return EFI_SUCCESS;
}
