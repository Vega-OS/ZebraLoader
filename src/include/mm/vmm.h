/*
 *  @description: vmm.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _MM_VMM_H_
#define _MM_VMM_H_

#include <def.h>

#define PTE_ADDR_MASK 0x000FFFFFFFFFF000
#define PTE_PRESENT (1ULL << 0)
#define PTE_WRITABLE (1ULL << 1)
#define PTE_USER (1ULL << 2)
#define PTE_NX (1ULL << 63)
#define PTE_GET_ADDR(VALUE) ((VALUE) & PTE_ADDR_MASK)
#define VMM_LOAD_CR3(cr3) \
  __asm__ __volatile__("mov %0, %%cr3" :: "r" (cr3) : "memory")

void vmm_init(void);


/*
 *  Maps a physical address `phys`
 *  to a virtual address `virt`
 *  with flags `flags`.
 *
 *  @pml4: Address space.
 */

void vmm_map_page(UINTN pml4, UINTN virt, UINTN phys,
                  UINTN flags);

/*
 *  Maps `count` pages.
 */

void vmm_map_pages(UINTN pml4, UINTN virt, UINTN phys,
                   UINTN flags, UINTN count);

/*
 *  Returns the pagemap to be used with the kernel.
 */

UINTN vmm_get_kernel_pml4(void);

#endif
