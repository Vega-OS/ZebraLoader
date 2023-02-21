/*
 *  @description: gop.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _DEV_GOP_H_
#define _DEV_GOP_H_

#include <def.h>

void gop_init(void);

/*
 *  Gets the index into the framebuffer
 *  from x and y values.
 */

UINT32 gop_get_index(UINT32 x, UINT32 y);

/*
 *  Returns the backbuffer address.
 */

UINT32* gop_get_addr(void);

/*
 *  Swaps the backbuffer and frontbuffer.
 */

void gop_swap_buffers(void);

/*
 *  Returns framebuffer width.
 */

UINT32 gop_get_width(void);

/* 
 *  Returns framebuffer height
 */

UINT32 gop_get_height(void);

/*
 *  Returns framebuffer pitch.
 */

UINT32 gop_get_pitch(void);

/*
 *  Frees the backbuffer and
 *  causes gop_get_addr to return
 *  the address of the front buffer.
 */

void gop_free_backbuffer(void);

#endif
