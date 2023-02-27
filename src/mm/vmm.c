/*
 *  @description: Virtual memory manager.
 *  @author: Ian Marco Moffett.
 */

#include <mm/vmm.h>
#include <mm/pmm.h>
#include <def.h>
#include <cpuid.h>
#include <string.h>

/*
 *  Invalidates a single page from
 *  the TLB.
 */

static inline void tlb_flush_single(UINTN vaddr)
{
  __asm("invlpg (%0)" :: "r" (vaddr) : "memory");
}

/*
 *  Gets the next level of
 *  a paging structure (returns virtual address).
 *
 *  @top_level: The structure that you want to get the next of (virtual)
 *  @index: Index into `top_level`.
 *  @alloc: 1 to allocate new structure if not present, 0 to return NULL.
 */

static UINTN* get_next_level(UINTN *top_level, UINTN index,
                             UINT8 alloc)
{
  if (top_level[index] & PTE_PRESENT)
  {
    UINTN phys = PTE_GET_ADDR(top_level[index]);
    return (UINTN *)phys;
  }

  if (!alloc)
  {
    return NULL;
  }

  UINTN phys = pmm_alloc_frame();
  memzero((UINTN *)phys, 4096);

  if (phys == 0)
  {
    Print(L"Failed to allocate frame\n");
    halt();
  }

  top_level[index] = phys
                     | PTE_PRESENT
                     | PTE_WRITABLE
                     | PTE_USER;

  return (UINTN *)phys;
}

void vmm_map_page(UINTN *pagemap, UINTN virt, UINTN phys,
                  UINTN flags, pagesize_t page_size)
{
  UINTN pml4_index = (virt >> 39) & 0x1FF;
  UINTN pdpt_index = (virt >> 30) & 0x1FF;
  UINTN pd_index   = (virt >> 21) & 0x1FF;
  UINTN pt_index   = (virt >> 12) & 0x1FF;
  
  UINTN *pdpt = get_next_level(pagemap, pml4_index, 1);
  UINTN *pd = get_next_level(pdpt, pdpt_index, 1);
  
  if (page_size == PAGESIZE_2MiB)
  {
    /* Map 2 MiB page if requested */
    pd[pd_index] = phys | flags | PTE_HUGE_PAGE;
    tlb_flush_single(virt);
    return;
  }
  
  UINTN *pt = get_next_level(pd, pd_index, 1);
  pt[pt_index] = phys | flags;
  tlb_flush_single(virt);
}

UINTN *vmm_new_pagemap(void)
{
  struct zebra_mmap mmap = pmm_get_mmap();
  UINTN *pagemap = (UINTN *)pmm_alloc_frame();
  memzero(pagemap, 4096);

  for (UINTN i = 0; i < mmap.entry_count; ++i)
  {
    struct zebra_mmap_entry *entry = &mmap.map[i];

    UINTN start = entry->phys_base;
    UINTN end = start + (entry->page_count*0x1000);
    for (UINTN j = start; j < end; j += 0x1000)
    {
      vmm_map_page(pagemap,
                   j,
                   j,
                   PTE_PRESENT | PTE_WRITABLE,
                   PAGESIZE_4K
      );
    }
  }
  

  for (UINTN i = 0; i < _2_MB*10; i += _2_MB)
  {
    vmm_map_page(pagemap,
                 i,
                 i,
                 PTE_PRESENT | PTE_WRITABLE,
                 PAGESIZE_2MiB
    );
  }

  return pagemap;
}
