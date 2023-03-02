/*
 *  @description: Vega protocol.
 *  @author: Ian Marco Moffett.
 */

#ifndef _PROTO_VEGA_H_
#define _PROTO_VEGA_H_

#include <def.h>
#include <mm/pmm.h>

struct vega_info
{
  /* Framebuffer related fields */
  UINT32 *fb;
  UINT32 fb_width;
  UINT32 fb_height;
  UINT32 fb_pitch;
  
  /* Physical memory map */
  struct zebra_mmap mmap;
};

#endif
