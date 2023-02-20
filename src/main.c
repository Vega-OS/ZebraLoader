/*
 *  @description: Bootloader entrypoint.
 *  @author: Ian Marco Moffett.
 */

#include <efi.h>
#include <printf.h>

EFI_BOOT_SERVICES* BS;
EFI_SYSTEM_TABLE* ST;

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  (void)ImageHandle;
  printf_init(SystemTable);
  clear_screen();

  ST = SystemTable;
  BS = ST->BootServices;
  
  printf("Press any key to boot\n");
  while (uefi_call_wrapper(BS->CheckEvent, 1, ST->ConIn->WaitForKey))
  {
    uefi_call_wrapper(BS->Stall, 1, 1000);
  }

  for (;;);
}
