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
#include <string.h>

#define HIGHER_HALF 0xFFFFFFFF80000000

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

/*
 *  Maps the framebuffer
 *  for the kernel.
 *  TODO: Get rid of this and use loader_map() instead 
 */

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

  info->mmap = pmm_get_mmap();
}


static void loader_map(UINTN virt, UINTN phys, UINTN *kernel_pagemap,
                        UINTN page_count, UINT32 flags)
{
  for (UINTN i = 0; i < page_count*0x1000; i += 0x1000)
  {
    UINTN vaddr = virt+i;
    UINTN paddr = phys+i;

    vmm_map_page(kernel_pagemap,
                 vaddr,
                 paddr,
                 flags,
                 PAGESIZE_4K
    );
  }
}

/*
 *  Returns a pointer to the
 *  list of section headers.
 */

static inline Elf64_Shdr *get_shdr(Elf64_Ehdr *eh)
{
  return (Elf64_Shdr *)((UINTN)eh + eh->e_shoff);
}

/*
 *  Returns an ELF section header
 *  from an index.
 */

static inline Elf64_Shdr *get_section(Elf64_Ehdr *eh, UINT32 idx)
{
  return &get_shdr(eh)[idx];
}


/*
 *  Initializes a section with
 *  type SHT_NOBITS.
 */

static void sht_nobits_init(UINTN *kernel_pagemap, Elf64_Ehdr *eh)
{
  for (UINT32 i = 0; i < eh->e_shnum; ++i)
  {
    Elf64_Shdr *section = get_section(eh, i);

    if (section->sh_type != SHT_NOBITS)
    {
      /* Skip if the section is present in the file */
      continue;
    }

    if (section->sh_size == 0)
    {
      /* Section is empty, skip it */
      continue;
    }

    if (section->sh_flags & SHF_ALLOC)
    {
      /* 
       * Get the page table entry flags, should be read-only
       * unless SHF_WRITE of section->sh_flags is set
       */
      uint32_t pte_flags = PTE_PRESENT;

      if (section->sh_flags & SHF_WRITE)
      {
        /* This section contains writable data */
        pte_flags |= PTE_WRITABLE;
      }

      UINTN tmp = pmm_alloc_frame();
      
      /* Map the memory */
      loader_map(section->sh_addr,
                 tmp,
                 kernel_pagemap,
                 ALIGN_UP(section->sh_size+1, 4096),
                 pte_flags
      );
    }
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
  UINTN phdrs_start = 0;
  
  map_framebuffer(kernel_pagemap);
  
  size = eh->e_phnum * eh->e_phentsize;
  phdrs = (void*)((UINTN)eh + eh->e_phoff);
  phdr = phdrs;
  phdrs_start = (UINTN)phdrs;

  /* Exit boot services and switch address spaces */
  struct zebra_mmap *mmap = pmm_get_mmap();
  uefi_call_wrapper(BS->ExitBootServices, 2, g_image_handle, mmap->efi_map_key);
  __asm("cli; mov %0, %%cr3" :: "r" ((UINTN)kernel_pagemap));

  /* Initialize memory for stuff like .bss if any */
  sht_nobits_init(kernel_pagemap, eh);
  
  /* Begin parsing the program headers */
  while ((UINTN)phdr < phdrs_start + size)
  {
    if (phdr->p_type == PT_LOAD)      /* Loadable segment */
    {
      UINTN page_count = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
      Elf64_Addr segment = phdr->p_vaddr;
      
      /* Ensure the segment is mapped */
      loader_map(segment,
                 pmm_alloc(page_count*4096),
                 kernel_pagemap,
                 page_count,
                 PTE_PRESENT | PTE_WRITABLE
      );

      ptr = (UINT8*)eh + phdr->p_offset;

      /* Ensure the segment is zero'd */
      _memset((void*)segment, 0x0, phdr->p_memsz);
      
      /* Copy to the segment */
      for (UINTN i = 0; i < phdr->p_filesz; ++i)
      {
        ((UINT8*)segment)[i] = ptr[i];
      }
    }

    phdr = (Elf64_Phdr*)((UINTN)phdr + eh->e_phentsize);
  }

  /* Load the kernel! */
  void(*kentry)(struct vega_info *);
  kentry = ((__attribute__((sysv_abi))void(*)(struct vega_info *))eh->e_entry);
  kentry(info);

  __builtin_unreachable();
}

void load_kernel(CHAR16 *file_name)
{
  /* Get file */
  EFI_FILE *elf_file = disk_get_file(file_name);

  if (!elf_file)
  {
    Print(L"Could not get file %s\n", file_name);
    halt();
  }

  /* Get file size */
  UINTN size;
  EFI_STATUS status = disk_get_size(elf_file, &size);

  if (EFI_ERROR(status))
  {
    Print(L"Could not get size of file %s\n", file_name);
    halt();
  }

  /* Allocate memory for file */
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

  /* Load file to memory */
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

  /* Parse elf header */
  Elf64_Ehdr *elf_hdr = (Elf64_Ehdr *)file_buffer;
  
  if (!is_eh_valid(elf_hdr))
  {
    Print(L"Kernel ELF header not valid!\n");
    halt();
  }

  do_load(elf_hdr);
}
