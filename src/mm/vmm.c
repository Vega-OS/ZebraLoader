/*
 *  @description: Virtual memory manager.
 *  @author: Ian Marco Moffett.
 */

#include <mm/vmm.h>
#include <mm/pmm.h>
#include <string.h>
#include <printf.h>

#define VIRT_TO_PT_INDEX(virt) ((virt >> 12) & 0x1FF)

static UINTN k_pml4 = 0;

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
    return NULL;
  }

  top_level[index] = phys
                     | PTE_PRESENT
                     | PTE_WRITABLE
                     | PTE_USER;

  return (UINTN*)phys;
}

static void __flush_tlb_single(UINTN virt)
{
  __asm__ __volatile__("invlpg (%0)" :: "r" (virt) : "memory");
}

void vmm_map_page(UINTN pml4, UINTN virt, UINTN phys,
                  UINTN flags)
{
  UINTN pml4_index = (virt >> 39) & 0x1FF;
  UINTN pdpt_index = (virt >> 30) & 0x1FF;
  UINTN pd_index   = (virt >> 21) & 0x1FF;

  UINTN* pdpt = get_next_level((UINTN*)pml4, pml4_index, 1);

  if (pdpt == NULL)
  {
    return;
  }

  UINTN* pd = get_next_level(pdpt, pdpt_index, 1);

  if (pd == NULL)
  {
    return;
  }

  UINTN* pt = get_next_level(pd, pd_index, 1);
  
  if (pt == NULL)
  {
    return;
  }

  pt[VIRT_TO_PT_INDEX(virt)] = phys | flags;
  __flush_tlb_single(virt);
}


void vmm_map_pages(UINTN pml4, UINTN virt, UINTN phys,
                   UINTN flags, UINTN count)
{
  for (UINTN i = virt; i < virt+(0x1000*count); i += 0x1000)
  {
    vmm_map_page(pml4, i, phys, flags);
    phys += 0x1000;
  }
}

UINTN vmm_get_kernel_pml4(void)
{
  return k_pml4;
}

void vmm_init(void)
{
  k_pml4 = pmm_alloc_frame();
  UINTN old = 0;
  
  if (k_pml4 == 0)
  {
    printf("Could not allocate memory for CR3.\n");
    halt();
  }

  /* Get the old PML4 */
  __asm__ __volatile__("mov %%cr3, %0" : "=r" (old) :: "memory");
  
  for (UINTN i = 0; i < 512; ++i)
  {
    ((UINTN*)k_pml4)[i] = ((UINTN*)old)[i];
  }
}
