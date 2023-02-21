/*
 *  @description: Loads the kernel.
 *  @author: Ian Marco Moffett.
 */

#include <loader.h>
#include <printf.h>
#include <elf.h>
#include <dev/disk.h>
#include <mm/vmm.h>
#include <mm/pmm.h>

#define HIGHER_HALF 0xC0000000

static UINT8 is_valid_elf(Elf64_Ehdr eh)
{
  return eh.e_ident[EI_CLASS] == ELFCLASS64
         && eh.e_ident[EI_DATA] == ELFDATA2LSB
         && eh.e_type == ET_EXEC
         && eh.e_machine == EM_X86_64;
}

__attribute__((noreturn)) void load_kernel(EFI_HANDLE image_handle)
{
  /* __KERNEL_ELF is set during compile time (as a flag) */
  EFI_FILE* file = disk_get_file(__KERNEL_ELF);
  if (file == NULL)
  {
    printf("Could not find kernel!\n");
    printf("Please check how you configured ZebraLoader.\n");
    halt();
  }

  Elf64_Ehdr eh;
  
  /* Read in the ELF header */
  UINTN size = sizeof(Elf64_Ehdr);
  file->Read(file, &size, &eh);

  if (!is_valid_elf(eh))
  {
    printf("Kernel ELF is invalid.\n");
    halt();
  }

  Elf64_Phdr* phdrs;
  file->SetPosition(file, eh.e_phoff);

  size = eh.e_phnum * eh.e_phentsize;
  BS->AllocatePool(EfiLoaderData, size, (void**)&phdrs);
  file->Read(file, &size, phdrs);

  Elf64_Phdr* phdr = phdrs;
  UINTN phdrs_start = (UINTN)phdr;
  
  /* Begin mapping the ELF */
  VMM_LOAD_CR3(vmm_get_kernel_pml4());
  while ((UINTN)phdr < phdrs_start + size)
  {
    if (phdr->p_type == PT_LOAD)
    {
      UINTN page_count = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
      Elf64_Addr segment = phdr->p_paddr;
       
      vmm_map_pages(vmm_get_kernel_pml4(), segment, segment-HIGHER_HALF,
                   PTE_PRESENT | PTE_WRITABLE, page_count);
      
      size = phdr->p_filesz;
      file->SetPosition(file, phdr->p_offset);
      file->Read(file, &size, (void*)segment);

      printf("%x\n", segment);
    }

    phdr = (Elf64_Phdr*)((UINTN)phdr + eh.e_phentsize);
  }
  
  /* We don't need UEFI boot services anymore */
  struct zebra_mmap mmap = pmm_get_mmap();  
  uefi_call_wrapper(BS->ExitBootServices, 2, image_handle, mmap.key); 
  
  /* Jump to the kernel entrypoint */
  int (*kentry)(void);
  kentry = ((__attribute__((sysv_abi)) int (*)(void)) eh.e_entry);
  kentry(); 

  __builtin_unreachable();
}
