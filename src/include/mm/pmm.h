/*
 *  @description: pmm.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _MM_PMM_H_
#define _MM_PMM_H_

#include <def.h>

typedef enum
{
  ZEBRA_MMAP_USABLE,
  ZEBRA_MMAP_ACPI_RECLAIM,
  ZEBRA_MMAP_RESERVED
} zebra_memtype_t;

struct zebra_mmap_entry
{
  zebra_memtype_t type;
  UINTN phys_base;
  UINTN page_count;
};

struct zebra_mmap
{
  struct zebra_mmap_entry* entries;
  UINTN entry_count;
  UINTN key;
};

void pmm_init(void);
UINTN pmm_alloc_frame(void);
struct zebra_mmap pmm_get_mmap(void);

#endif
