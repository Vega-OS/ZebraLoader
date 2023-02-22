/*
 *  @description: Physical memory manager.
 *  @author: Ian Marco Moffett.
 */

#include <mm/pmm.h>

#define NEXT_MEMORY_DESCRIPTOR(mem_desc, size) \
  ((EFI_MEMORY_DESCRIPTOR *)((UINT8 *)(mem_desc) + (size)))

static struct zebra_mmap mmap;

/*
 *  Parses the EFI memory map
 *  and fills the zebra memory map.
 */

static void parse_efi_mmap(EFI_MEMORY_DESCRIPTOR* efi_mmap,
                           UINTN efi_mmap_size, UINTN efi_descriptor_size)
{
  EFI_MEMORY_DESCRIPTOR* entry = efi_mmap;
  mmap.map = AllocatePool(efi_mmap_size);
  UINTN i = 0;
  UINTN total_bytes = 0;
  UINTN total_mib = 0;

  do
  {
    entry = NEXT_MEMORY_DESCRIPTOR(entry, efi_descriptor_size);
    switch (entry->Type)
    {
      case EfiConventionalMemory:
        mmap.map[i].type = ZEBRA_MEM_USABLE;
        break;
      case EfiACPIReclaimMemory:
        mmap.map[i].type = ZEBRA_MEM_ACPI_RECLAIMABLE;
        break;
      default:
        mmap.map[i].type = ZEBRA_MEM_RESERVED;
        break;
    }

    mmap.map[i].phys_base = entry->PhysicalStart;
    mmap.map[i].page_count = entry->NumberOfPages;
    total_bytes += entry->NumberOfPages * 4096;
  } while ((UINT8*)entry < (UINT8*)efi_mmap + efi_mmap_size);
  
  total_mib = total_bytes/1048576; 
  if (total_mib < 1024)
  {
    Print(L"System has a total of %d MiB of memory.\n", total_mib);
  }
  else
  {
    Print(L"System has a total of %d GiB of memory.\n", total_mib/1024);
  }
}

static void init_mmap(void)
{
  EFI_STATUS status;
  EFI_MEMORY_DESCRIPTOR* efi_mmap = NULL;
  UINTN efi_mmap_size = 0;
  UINTN efi_map_key;
  UINTN efi_descriptor_size;
  UINT32 efi_descriptor_version;
  
  /* Fetch the EFI memory map size */
  status = uefi_call_wrapper(BS->GetMemoryMap,
                             5,
                             &efi_mmap_size,
                             efi_mmap,
                             &efi_map_key,
                             &efi_descriptor_size,
                             &efi_descriptor_version
  );

  if (status != EFI_BUFFER_TOO_SMALL)
  {
    Print(L"Failed to fetch memory map size.\n");
    halt();
  }

  efi_mmap = AllocatePool(efi_mmap_size + 2 * efi_descriptor_size);

  if (efi_mmap == NULL)
  {
    Print(L"Not enough memory to hold the EFI memory map!\n");
    halt();
  }

  /* Actually fetch the EFI memory map now */
  efi_mmap_size = efi_mmap_size + 2 * efi_descriptor_size;
  status = uefi_call_wrapper(BS->GetMemoryMap,
                             5,
                             &efi_mmap_size,
                             efi_mmap,
                             &efi_map_key,
                             &efi_descriptor_size,
                             &efi_descriptor_version
  );

  if (EFI_ERROR(status))
  {
    Print(L"Failed to fetch EFI memory map.\n");
    halt();
  }

  parse_efi_mmap(efi_mmap, efi_mmap_size, efi_descriptor_size);
}

struct zebra_mmap pmm_get_mmap(void)
{
  return mmap;
}

void pmm_init(void)
{
  init_mmap();
}
