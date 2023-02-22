/*
 *  @description: Kernel loading.
 *  @author: Ian Marco Moffett.
 */

#include <loader.h>
#include <elf.h>
#include <dev/disk.h>
#include <mm/vmm.h>
#include <mm/pmm.h>

#define HIGHER_HALF 0xC0000000

/*
 *  Returns 1 if the ELF header
 *  is valid, otherwise 0.
 */

static UINT8 is_valid_elf(Elf64_Ehdr eh)
{
  return eh.e_ident[EI_CLASS] == ELFCLASS64
         && eh.e_ident[EI_DATA] == ELFDATA2LSB
         && eh.e_type == ET_EXEC
         && eh.e_machine == EM_X86_64;
}

void load_kernel(EFI_HANDLE image_handle)
{
  UINTN* kernel_pagemap = vmm_new_pagemap();
  EFI_FILE* kernel = disk_get_file(__KERNEL_ELF);
  if (kernel == NULL)
  {
    Print(L"Could not load %s\n", __KERNEL_ELF);
    halt();
  }

  Elf64_Ehdr eh;
  UINTN size = sizeof(Elf64_Ehdr);
  uefi_call_wrapper(kernel->Read, 3,
                    kernel,
                    &size,
                    &eh
  );

  if (!is_valid_elf(eh))
  {
    Print(L"Kernel ELF is invalid.\n");
    halt();
  }

  uefi_call_wrapper(kernel->SetPosition, 2, kernel, eh.e_phoff);
  size = eh.e_phnum*eh.e_phentsize; 

  Elf64_Phdr* phdrs = AllocatePool(size);
  uefi_call_wrapper(kernel->Read, 3, kernel, &size, phdrs);

  Elf64_Phdr* phdr = phdrs;
  UINTN phdrs_start = (UINTN)phdrs;
  
  while ((UINTN)phdr < phdrs_start + size)
  {
    if (phdr->p_type == PT_LOAD)
    {
      UINTN page_count = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
      Elf64_Addr segment = phdr->p_paddr;
      Print(L"%x\n", segment);
      vmm_map_pages(kernel_pagemap,
                    segment,
                    segment - HIGHER_HALF,
                    PTE_PRESENT | PTE_WRITABLE,
                    PAGESIZE_4K,
                    page_count
      );
    }

    phdr = (Elf64_Phdr*)((UINTN)phdr + eh.e_phentsize);
  }

  /* We don't need UEFI boot services anymore */
  __asm("mov %0, %%cr3" :: "r" ((uintptr_t)kernel_pagemap));
  // Print(L"Fun\n");
  __asm("cli; hlt");

  struct zebra_mmap mmap = pmm_get_mmap();
  uefi_call_wrapper(BS->ExitBootServices, 2, image_handle, mmap.efi_map_key);

  void(*kentry)(void);
  kentry = ((__attribute__((sysv_abi))void(*)(void))eh.e_entry);
  
  __asm("cli; hlt");

  kentry();
  __builtin_unreachable();
}
