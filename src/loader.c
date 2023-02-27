/*
 * @description: ELF file loader
 * @author: Queso Fuego
 */

#include <dev/disk.h>
#include <proto/vega.h>
#include <dev/gop.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <elf.h>
#include <loader.h>

/*
 *  Returns 1 if the ELF header
 *  passed is valid.
 */

static UINT8 is_eh_valid(Elf64_Ehdr *eh)
{
  return eh->e_ident[EI_CLASS] == ELFCLASS64
         && eh->e_ident[EI_DATA] == ELFDATA2LSB
         && eh->e_type == ET_EXEC
         && eh->e_machine == EM_X86_64;
}

static void map_framebuffer(UINTN *kernel_pagemap)
{
  UINT32 fb_size = gop_get_pitch()*gop_get_height();
  UINTN fb_base = (UINTN)gop_get_addr();

  for (UINTN i = fb_base; i < fb_base+fb_size; i += _2_MB)
  {
    vmm_map_page(kernel_pagemap,
                 i,
                 i,
                 PTE_PRESENT | PTE_WRITABLE,
                 PAGESIZE_2MiB
    );
  }
}

static void init_proto(struct vega_info *info)
{
  gop_destroy_backbuffer();
  info->fb = gop_get_addr();
  info->fb_width = gop_get_width();
  info->fb_height = gop_get_height();
  info->fb_pitch = gop_get_pitch(); 
}

static void map_segment(Elf64_Addr segment, UINTN *kernel_pagemap,
                        UINTN page_count)
{
  for (UINTN i = 0; i < page_count*0x1000; i += 0x1000)
  {
    UINTN addr = segment+i;
    vmm_map_page(kernel_pagemap,
                 addr,
                 (UINTN)AllocatePool(0x1000),
                 PTE_PRESENT | PTE_WRITABLE,
                 PAGESIZE_4K
    );
  }
}

/*
 *  Does the process of loading the kernel.
 */

static void do_load(Elf64_Ehdr *eh)
{
  struct vega_info *info = AllocatePool(sizeof(struct vega_info));
  init_proto(info);

  UINTN *kernel_pagemap = vmm_new_pagemap();
  Elf64_Phdr *phdrs = NULL;
  Elf64_Phdr *phdr = NULL;
  UINT8 *ptr = NULL;
  UINTN size;

  map_framebuffer(kernel_pagemap);
  
  size = eh->e_phnum * eh->e_phentsize; 
  phdrs = (void*)((UINTN)eh + eh->e_phoff);   // Start of phdrs.
  phdr = phdrs;

  UINTN phdrs_start = (UINTN)phdrs;

  __asm("mov %0, %%cr3" :: "r" ((UINTN)kernel_pagemap));  // Switch vaddrsp.
  while ((UINTN)phdr < phdrs_start + size)
  {
    if (phdr->p_type == PT_LOAD)      // Loadable segment.
    {
      UINTN page_count = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
      Elf64_Addr segment = phdr->p_vaddr;
      map_segment(segment, kernel_pagemap, page_count);

      ptr = (UINT8*)eh + phdr->p_offset;
      for (UINTN i = 0; i < phdr->p_filesz; ++i)
      {
        ((UINT8*)segment)[i] = ptr[i];
      }
    }

    phdr = (Elf64_Phdr*)((UINTN)phdr + eh->e_phentsize);
  }

  struct zebra_mmap mmap = pmm_get_mmap();
  uefi_call_wrapper(BS->ExitBootServices, 2, g_image_handle, mmap.efi_map_key);

  void(*kentry)(struct vega_info *);
  kentry = ((__attribute__((sysv_abi))void(*)(struct vega_info *))eh->e_entry);

  kentry(info);
  __builtin_unreachable();
}

void load_kernel(CHAR16 *file_name)
{
  // Get file
  EFI_FILE *elf_file = disk_get_file(file_name);
  if (!elf_file)
  {
    Print(L"Could not get file %s\n", file_name);
    halt();
  }

  // Get file size
  UINTN size;
  EFI_STATUS status = disk_get_size(elf_file, &size);
  if (EFI_ERROR(status))
  {
    Print(L"Could not get size of file %s\n", file_name);
    halt();
  }

  // Allocate memory for file
  VOID *file_buffer;
  status = uefi_call_wrapper(BS->AllocatePool, 3,
                             EfiLoaderData,
                             size,
                             &file_buffer
  );

  if (EFI_ERROR(status))
  {
    Print(L"Could not allocate memory for file %s\n", file_name);
    halt();
  }

  // Load file to memory
  status = uefi_call_wrapper(elf_file->Read, 3, 
                             elf_file,
                             &size,
                             file_buffer
  );

  if (EFI_ERROR(status))
  {
    Print(L"Could not load file %s to memory\n", file_name);
    halt();
  }

  // Parse elf header
  Elf64_Ehdr *elf_hdr = (Elf64_Ehdr *)file_buffer;

  if (!is_eh_valid(elf_hdr))
  {
    Print(L"Kernel ELF header not valid!\n");
    halt();
  }

  do_load(elf_hdr);
}

