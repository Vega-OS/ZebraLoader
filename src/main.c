/*
 *  @description: Bootloader entrypoint.
 *  @author: Ian Marco Moffett.
 */

#include <efi.h>
#include <printf.h>
#include <menu.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <dev/gop.h>

EFI_BOOT_SERVICES* BS;
EFI_RUNTIME_SERVICES* RT;
EFI_SYSTEM_TABLE* ST;

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  (void)ImageHandle;
  printf_init(SystemTable);
  clear_screen();

  ST = SystemTable;
  BS = ST->BootServices;
  RT = ST->RuntimeServices; 

  clear_screen();
  pmm_init();
  vmm_init();

  gop_init();
  menu_start();

  for (;;);
}
