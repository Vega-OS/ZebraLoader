/*
 *  @description: Virtual memory manager.
 *  @author: Ian Marco Moffett.
 */

#include <mm/vmm.h>
#include <mm/pmm.h>
#include <string.h>
#include <printf.h>

void vmm_init(void)
{
  UINTN cr3 = pmm_alloc_frame();

  if (cr3 == 0)
  {
    printf("Could not allocate memory for CR3.\n");
    halt();
  }
  
  /* Memory initially is identity mapped */
  memset((void*)cr3, 0, 4096);
}
