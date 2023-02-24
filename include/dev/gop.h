#ifndef _DEV_GOP_H_
#define _DEV_GOP_H_

#include <def.h>

void gop_init(void);

/*
 *  Returns the size of the framebuffer
 *  in bytes.
 */

UINT32 gop_get_size(void);

/*
 *  Returns the index into the
 *  framebuffer.
 */

UINT32 gop_get_index(UINT32 x, UINT32 y);

/*
 *  Returns the width of the framebuffer.
 */

UINT32 gop_get_width(void);

/*
 *  Returns the height of the framebuffer.
 */

UINT32 gop_get_height(void);

/*
 *  Returns the address of the backbuffer.
 */

UINT32* gop_get_addr(void);

/*
 *  Returns framebuffer pitch.
 */

UINT32 gop_get_pitch(void);

/*
 *  Writes the backbuffer onto
 *  the main buffer.
 */

void gop_swap_buffers(void);

/*
 *  Writes the backbuffer from
 *  postion (start_x,start_y) to
 *  the main buffer until (end_x, end_y)
 */

void gop_swap_buffers_at(UINT32 start_x, UINT32 start_y,
                         UINT32 end_x, UINT32 end_y);

#endif
