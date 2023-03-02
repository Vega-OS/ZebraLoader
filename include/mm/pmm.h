/*
 *  @description: pmm.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _MM_PMM_H_
#define _MM_PMM_H_

#include <def.h>

typedef enum
{
  ZEBRA_MEM_USABLE,
  ZEBRA_MEM_ACPI_RECLAIMABLE,
  ZEBRA_MEM_RESERVED
} zebra_mem_type_t;

struct zebra_mmap_entry
{
  UINTN phys_base;
  UINTN length_bytes;
  zebra_mem_type_t type;
};

struct zebra_mmap
{
  struct zebra_mmap_entry *map;
  UINTN entry_count;

  /* Internal EFI related fields */
  UINTN efi_map_key;
  UINTN efi_descriptor_size;
  UINTN efi_descriptor_version;
};

void pmm_init(void);
UINTN pmm_alloc_frame(void);
UINTN pmm_alloc(UINTN byte_count);
struct zebra_mmap *pmm_get_mmap(void);

#endif
