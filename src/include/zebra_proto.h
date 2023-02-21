/*
 *  @description: Zebra protocol definitions.
 *  @author: Ian Marco Moffett.
 */

#ifndef _ZEBRA_PROTO_H_
#define _ZEBRA_PROTO_H_

#include <mm/pmm.h>
#include <def.h>

struct framebuffer_info
{
  UINTN base_addr;
  UINTN pitch;
  UINTN width;
  UINTN height;
};

struct zebra_info
{
  struct zebra_mmap mmap;
  struct framebuffer_info fbinfo;

  /* Routines */
  void(*shutdown)(void);
};

#endif
