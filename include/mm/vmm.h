/*
 *  @description: vmm.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _MM_VMM_H_
#define _MM_VMM_H_

#include <def.h>

/* Page translation table entry bits */
#define PTE_PRESENT   (1 << 0)
#define PTE_WRITABLE  (1 << 1)
#define PTE_USER      (1 << 2)
#define PTE_HUGE_PAGE (1 << 7)
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000
#define PTE_GET_ADDR(VALUE) ((VALUE) & PTE_ADDR_MASK)
#define _2_MB (0x100000*2)

typedef enum
{
  PAGESIZE_2MiB,
  PAGESIZE_1GiB,
  PAGESIZE_4K
} pagesize_t;

/*
 *  Maps a virtual address to
 *  a physical address.
 *
 *  @pagemap: (virtual address)
 *  @virt: Virtual address.
 *  @phys: Physical address.
 *  @flags: PTE flags.
 *  @page_size: Size of page.
 */

void vmm_map_page(UINTN* pagemap, UINTN virt, UINTN phys,
                  UINTN flags, pagesize_t page_size);

/*
 *  Creates a new pagemap.
 */

UINTN* vmm_new_pagemap(void);

#endif
