#ifndef _PROTO_VEGA_H_
#define _PROTO_VEGA_H_

#include <def.h>

struct vega_info
{
  /* Framebuffer related fields */
  UINT32 *fb;
  UINT32 fb_width;
  UINT32 fb_height;
  UINT32 fb_pitch;
};

#endif
