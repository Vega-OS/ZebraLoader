/*
 *  @description: Virtual memory manager.
 *  @author: Ian Marco Moffett.
 */

#include <mm/vmm.h>
#include <mm/pmm.h>
#include <cpuid.h>

static inline void __flush_tlb_single(UINTN virt)
{
  __asm("invlpg (%0)" :: "r" (virt) : "memory");
}

static UINT8 is_1gib_page_supported(void)
{
  static UINT8 has_cache = 0;
  static UINT8 cache_val = 0;

  if (has_cache)
  {
    return cache_val;
  }
  
  UINT32 unused, edx;
  __cpuid(0x80000001, unused, unused, unused, edx);

  cache_val = (edx & (1 << 26)) != 0;
  has_cache = 1;
  return cache_val;
}

/*
 *  Get's the next level of
 *  a paging structure (returns virtual address).
 *
 *  @top_level: The structure that you want to get the next of (virtual)
 *  @index: Index into `top_level`.
 *  @alloc: 1 to allocate new structure if not present, 0 to return NULL.
 */

static UINTN* get_next_level(UINTN* top_level, UINTN index,
                             UINT8 alloc)
{
  if (top_level[index] & PTE_PRESENT)
  {
    UINTN phys = PTE_GET_ADDR(top_level[index]);
    return (UINTN*)phys;
  }

  if (!alloc)
  {
    return NULL;
  }

  UINTN phys = pmm_alloc_frame();

  if (phys == 0)
  {
    Print(L"Failed to allocate frame in %s()", __func__);
    halt();
  }

  top_level[index] = phys
                     | PTE_PRESENT
                     | PTE_WRITABLE
                     | PTE_USER;

  return (UINTN*)phys;
}

void vmm_map_page(UINTN* pagemap, UINTN virt, UINTN phys,
                  UINTN flags, pagesize_t page_size)
{
  UINTN pml4_index = (virt >> 39) & 0x1FF;
  UINTN pdpt_index = (virt >> 30) & 0x1FF;
  UINTN pd_index   = (virt >> 21) & 0x1FF;
  UINTN pt_index   = (virt >> 12) & 0x1FF;

  UINTN* pdpt = get_next_level(pagemap, pml4_index, 1); 
  if (is_1gib_page_supported() && page_size == PAGESIZE_1GiB)
  {
    pdpt[pdpt_index] = phys | flags | PTE_HUGE_PAGE;
    __flush_tlb_single(virt);
    return;
  }

  if (page_size == PAGESIZE_1GiB)
  {
    /* 
     *  1GiB pages are not supported,
     *  map 1GiB with 2 MiB pages instead.
     */

    for (UINTN i = 0; i < 0x40000000; i += 200000)
    {
      vmm_map_page(pagemap,
                   virt + i,
                   phys + i,
                   flags,
                   PAGESIZE_2MiB
      );
    }
    
    __flush_tlb_single(virt);
    return;
  }

  UINTN* pd = get_next_level(pdpt, pdpt_index, 1);
  if (page_size == PAGESIZE_2MiB)
  {
    pd[pd_index] = phys | flags | PTE_HUGE_PAGE;
    __flush_tlb_single(virt);
    return;
  }

  UINTN* pt = get_next_level(pd, pd_index, 1);
  pt[pt_index] = phys | flags;
  __flush_tlb_single(virt);
}


UINTN* vmm_new_pagemap(void)
{
  UINTN* pagemap = (UINTN*)pmm_alloc_frame();
  struct zebra_mmap mmap = pmm_get_mmap();

  vmm_map_page(pagemap,
               0x200000,
               0x200000,
               PTE_PRESENT | PTE_WRITABLE, PAGESIZE_2MiB
  );

  for (UINTN i = 0x200000; i < 0x40000000; i += 0x200000)
  {
    vmm_map_page(pagemap, i, i, PTE_PRESENT | PTE_WRITABLE, PAGESIZE_2MiB);
  }

  return pagemap;
}
