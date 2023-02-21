/*
 *  @description: Physical memory manager.
 *  @author: Ian Marco Moffett.
 */

#include <mm/pmm.h>
#include <printf.h>
#include <string.h>

#define NEXT_MEMORY_DESCRIPTOR(ptr, size)  \
  ((EFI_MEMORY_DESCRIPTOR*) (((UINT8*) ptr) + size))

static struct zebra_mmap mmap;
static struct zebra_mmap_entry* free_mmap_entry = NULL;

/*
 *  Gets the zebra memory map
 *  from the EFI memory map.
 *
 *  @efi_mmap: EFI memory map.
 *  @n: EFI memory map entries.
 */

static struct zebra_mmap get_zebra_mmap(EFI_MEMORY_DESCRIPTOR* efi_mmap,
                                               size_t mmap_size, size_t desc_sz)
{
  struct zebra_mmap mmap;
  EFI_STATUS s;
  UINTN entry_count = 0;
  mmap.entries = NULL;

  /* Allocate a pool of memory for our memory map */
  s = uefi_call_wrapper(BS->AllocatePool, 2,
                        EfiLoaderData,
                        mmap_size,
                        (void**)&mmap.entries);

  if (s != 0)
  {
    printf("Failed to allocate pool for zebra mmap (status=%x)\n", s);
    halt();
  }
  
  EFI_MEMORY_DESCRIPTOR* entry = efi_mmap;
  do
  {
    switch (entry->Type)
    {
      case EfiConventionalMemory:
        mmap.entries[entry_count].type = ZEBRA_MMAP_USABLE;
        break;
      case EfiACPIReclaimMemory:
        mmap.entries[entry_count].type = ZEBRA_MMAP_ACPI_RECLAIM;
        break;
      default:
        mmap.entries[entry_count].type = ZEBRA_MMAP_RESERVED;
        break;
    }
    mmap.entries[entry_count].phys_base = entry->PhysicalStart;
    mmap.entries[entry_count].page_count = entry->NumberOfPages;
    ++entry_count;

    entry = NEXT_MEMORY_DESCRIPTOR(entry, desc_sz);
  } while ((UINTN)entry < (UINTN)efi_mmap + mmap_size);
  
  mmap.entry_count = entry_count;
  return mmap;
}

static struct zebra_mmap get_mmap(void)
{
  EFI_MEMORY_DESCRIPTOR* efi_mmap = NULL;
  UINTN efi_mmap_size = 0;
  UINTN efi_mmap_key = 0;
  UINTN efi_mmap_desc_size = 0;
  UINT32 efi_mmap_desc_version = 0;
  EFI_STATUS s = 0;
  
  /* Get the memory map size */
  s = uefi_call_wrapper(BS->GetMemoryMap, 5,
                        &efi_mmap_size,
                        efi_mmap,
                        &efi_mmap_key,
                        &efi_mmap_desc_size,
                        &efi_mmap_desc_version);

  if (s != EFI_BUFFER_TOO_SMALL)
  {
    printf("Error getting memory map.\n");
    halt();
  }

  /* Allocate memory for our memory map */
  uefi_call_wrapper(BS->AllocatePool, 3,
                    EfiLoaderData,
                    efi_mmap_size + 2 * efi_mmap_desc_size,
                    (void**)&efi_mmap);

  if (efi_mmap == NULL)
  {
    printf("Failed to allocate memory for the EFI memory map.\n");
    halt();
  }
  
  /* Get the memory map */
  efi_mmap_size = efi_mmap_size + 2 * efi_mmap_desc_size;
  s = uefi_call_wrapper(BS->GetMemoryMap, 5,
                        &efi_mmap_size,
                        efi_mmap,
                        &efi_mmap_key,
                        &efi_mmap_desc_size,
                        &efi_mmap_desc_version);

  if (EFI_ERROR(s))
  {
    printf("Error getting memory map.\n");
    halt();
  }
  
  return get_zebra_mmap(efi_mmap, efi_mmap_size, efi_mmap_desc_size);
}


UINTN pmm_alloc_frame(void)
{
  if (free_mmap_entry->page_count == 0)
  {
    return 0;
  }

  uintptr_t alloc = free_mmap_entry->phys_base;
  free_mmap_entry->phys_base += 4096;
  --free_mmap_entry->page_count;
  return alloc;
}

void pmm_init(void)
{
  mmap = get_mmap();
  static size_t n_bytes = 1;
  static size_t memory_size_mib = 0;
  
  /* Compute how much memory there is */
  for (size_t i = 0; i < mmap.entry_count; ++i)
  {
    struct zebra_mmap_entry* entry = &mmap.entries[i]; 
    n_bytes += entry->page_count*4096;

    if (entry->type == ZEBRA_MMAP_USABLE && free_mmap_entry == NULL)
    {
      /* Set free_mmap_entry to this memory that we can use */
      free_mmap_entry = entry;
    }
  }
  
  memory_size_mib = n_bytes/1048576;    /* 1 MiB => 1048576 bytes */
   
  if (memory_size_mib < 1024)
  {
    printf("Detected %d MiB of memory.\n", memory_size_mib);
  }
  else
  {
    /* 1 GiB => 1024 MiB */
    printf("Detected %d GiB of memory.\n", memory_size_mib/1024);
  }
}
