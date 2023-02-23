/*
 *  @description: Kernel loading.
 *  @author: Ian Marco Moffett.
 */

#include <loader.h>
#include <elf.h>
#include <dev/disk.h>
#include <mm/vmm.h>
#include <mm/pmm.h>

#define HIGHER_HALF 0xFFFFFFFF80000000

/*
 *  Returns 1 if the ELF header
 *  is valid, otherwise 0.
 */

static UINT8 is_valid_elf(Elf64_Ehdr* eh)
{
  return eh->e_ident[EI_CLASS] == ELFCLASS64
         && eh->e_ident[EI_DATA] == ELFDATA2LSB
         && eh->e_type == ET_EXEC
         && eh->e_machine == EM_X86_64;
}

static void map_page(Elf64_Addr vaddr, UINTN* kernel_pagemap,
                     UINT64 n_pages)
{
  for (UINTN i = 0; i < n_pages*0x1000; i += 0x1000)
  {
    UINTN addr = vaddr+i;
    vmm_map_page(kernel_pagemap,
                  addr,
                  (UINTN)AllocatePool(0x1000),
                  PTE_PRESENT | PTE_WRITABLE,
                  PAGESIZE_4K
    );
  }
}

void load_kernel(EFI_HANDLE image_handle)
{
  EFI_FILE* kernel = disk_get_file(__KERNEL_ELF);
  UINTN* kernel_pagemap = vmm_new_pagemap();
  UINTN elf_size = 0;
  UINT8* kernel_elf = NULL;

  disk_get_file_size(kernel, &elf_size);

  if (kernel == NULL || elf_size == 0)
  {
    Print(L"Could not load %s\n", __KERNEL_ELF);
    halt();
  }

  kernel_elf = AllocatePool(elf_size);

  uefi_call_wrapper(kernel->Read, 3,
                    kernel,
                    &elf_size,
                    kernel_elf
  );
  
  Elf64_Ehdr* eh = (Elf64_Ehdr*)kernel_elf;
  if (!is_valid_elf(eh))
  {
    Print(L"Kernel ELF is invalid.\n");
    halt();
  }
   
  UINTN size = eh->e_phnum*eh->e_phentsize; 
  Elf64_Phdr* phdrs = (VOID*)(kernel_elf + eh->e_phoff);

  Elf64_Phdr* phdr = phdrs;
  UINTN phdrs_start = (UINTN)phdrs;
  UINT8* ptr = NULL;
  
  __asm("mov %0, %%cr3" :: "r" ((UINTN)kernel_pagemap)); 
  while ((UINTN)phdr < phdrs_start + size)
  {
    if (phdr->p_type == PT_LOAD)
    {
      /* Map the segment so we can access it */
      UINTN page_count = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
      Elf64_Addr segment = phdr->p_vaddr;
      map_page(segment, kernel_pagemap, page_count);
      
      ptr = (UINT8*)kernel_elf + phdr->p_offset;
      for (UINTN i = 0; i < phdr->p_filesz; ++i)
      {
        ((UINT8*)segment)[i] = ptr[i];
      }
    }

    phdr = (Elf64_Phdr*)((UINTN)phdr + eh->e_phentsize);
  }

  /* We don't need UEFI boot services anymore */
  struct zebra_mmap mmap = pmm_get_mmap();
  uefi_call_wrapper(BS->ExitBootServices, 2, image_handle, mmap.efi_map_key); 

  /* Pass control to the kernel */ 
  UINTN kentry = eh->e_entry;  
  __asm("cli; jmp *%0" :: "r" (kentry));
  __builtin_unreachable();
}
