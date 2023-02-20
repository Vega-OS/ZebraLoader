@echo -off
mode 80 25

cls
if exist .\efi\boot\BOOTX64.EFI then
 .\efi\boot\BOOTX64.EFI
 goto END
endif

if exist fs0:\efi\boot\BOOTX64.EFI then
 fs0:
 efi\boot\BOOTX64.EFI
 goto END
endif

if exist fs1:\efi\boot\BOOTX64.EFI then
 fs1:
 efi\boot\BOOTX64.EFI
 goto END
endif

if exist fs2:\efi\boot\BOOTX64.EFI then
 fs2:
 efi\boot\BOOTX64.EFI
 goto END
endif

if exist fs3:\efi\boot\BOOTX64.EFI then
 fs3:
 efi\boot\BOOTX64.EFI
 goto END
endif

if exist fs4:\efi\boot\BOOTX64.EFI then
 fs4:
 efi\boot\BOOTX64.EFI
 goto END
endif

if exist fs5:\efi\boot\BOOTX64.EFI then
 fs5:
 efi\boot\BOOTX64.EFI
 goto END
endif

if exist fs6:\efi\boot\BOOTX64.EFI then
 fs6:
 efi\boot\BOOTX64.EFI
 goto END
endif

if exist fs7:\efi\boot\BOOTX64.EFI then
 fs7:
 efi\boot\BOOTX64.EFI
 goto END
endif

:END
