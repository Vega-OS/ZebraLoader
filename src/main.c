#include <def.h>

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
  
  return EFI_SUCCESS;
}
