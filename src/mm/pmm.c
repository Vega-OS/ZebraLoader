/*
 *  @description: Physical memory manager.
 *  @author: Ian Marco Moffett.
 */

#include <mm/pmm.h>

#define NEXT_MEMORY_DESCRIPTOR(mem_desc, size) \
  ((EFI_MEMORY_DESCRIPTOR *)((UINT8 *)(mem_desc) + (size)))

static struct zebra_mmap mmap;
static struct zebra_mmap_entry* usable_entry = NULL;

static void align_mmap_entry(struct zebra_mmap_entry *entry)
{
  entry->phys_base = ALIGN_UP(entry->phys_base, 4096);
  entry->page_count = ALIGN_DOWN(entry->page_count*4096, 4096)/4096;
}

/*
 *  Parses the EFI memory map
 *  and fills the zebra memory map.
 */

static void parse_efi_mmap(EFI_MEMORY_DESCRIPTOR* efi_mmap,
                           UINTN efi_mmap_size, UINTN efi_descriptor_size)
{
  EFI_MEMORY_DESCRIPTOR *entry = efi_mmap;
  mmap.map = AllocatePool(efi_mmap_size);
  mmap.entry_count = 0;
  UINTN total_bytes = 0;
  UINTN total_mib = 0;

  do
  {
    switch (entry->Type)
    {
      case EfiConventionalMemory:
        mmap.map[mmap.entry_count].type = ZEBRA_MEM_USABLE;
        break;
      case EfiACPIReclaimMemory:
        mmap.map[mmap.entry_count].type = ZEBRA_MEM_ACPI_RECLAIMABLE;
        break;
      default:
        mmap.map[mmap.entry_count].type = ZEBRA_MEM_RESERVED;
        break;
    }

    mmap.map[mmap.entry_count].phys_base = entry->PhysicalStart;
    mmap.map[mmap.entry_count].page_count = entry->NumberOfPages;
    align_mmap_entry(&mmap.map[mmap.entry_count++]);

    total_bytes += entry->NumberOfPages * 4096;
    entry = NEXT_MEMORY_DESCRIPTOR(entry, efi_descriptor_size);
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
  EFI_MEMORY_DESCRIPTOR *efi_mmap = NULL;
  UINTN efi_mmap_size = 0;
  UINTN efi_map_key;
  UINTN efi_descriptor_size;
  UINT32 efi_descriptor_version;
  
  // Fetch the EFI memory map size.
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

  // Actually fetch the EFI memory map now.
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
  FreePool(efi_mmap);
}

static struct zebra_mmap_entry *get_free_segment(void)
{
  for (UINTN i = 0; i < mmap.entry_count; ++i)
  {
    struct zebra_mmap_entry *entry = &mmap.map[i];
    if (entry->type == ZEBRA_MEM_USABLE)
    {
      return entry;
    }
  }

  return NULL;
}

struct zebra_mmap pmm_get_mmap(void)
{
  return mmap;
}

UINTN pmm_alloc_frame(void)
{
  struct zebra_mmap_entry *usable_segment = get_free_segment();
  UINTN ret = usable_segment->phys_base;
  usable_segment->phys_base += 4096;
  --usable_segment->page_count;
  
  if (usable_segment->page_count == 0)
  {
    usable_segment->type = ZEBRA_MEM_RESERVED;
  }

  return ret;
}

void pmm_init(void)
{
  init_mmap();
}
