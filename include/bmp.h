
/*
 *  @description: bmp.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _BMP_H_
#define _BMP_H_

#include <def.h>

/*
 *  @signature: 'BM'
 *  @file_size: File size in bytes.
 *  @reserved: Zero.
 *  @data_offset: Offset to raster data.
 *
 *  @size: Size of info header.
 *  @width: Bitmap width.
 *  @height: Bitmap height.
 *  @planes: Number of planes (1)
 *  @bit_count: Bits per pixel.
 *  @compression: Type of compression (0 = none)
 *  @image_size: Size of image.
 *  @x_pixels_per_m: Horizontal resolution.
 *  @y_pixels_per_m: Vertical resolution.
 *  @colors_used: Number of actually used colors.
 *  @colors_important: Number of important colors (0 = all)
 */

struct bmp_header
{
  /* Header */
  UINT16 signature;
  UINT32 file_size;
  UINT32 reserved;
  UINT32 data_offset;

  /* Info header */
  UINT32 size;
  UINT32 width;
  UINT32 height;
  UINT16 planes;
  UINT16 bit_count;
  UINT32 compression;
  UINT32 image_size;
  UINT32 x_pixels_per_m;
  UINT32 y_pixels_per_m;
  UINT32 colors_used;
  UINT32 colors_important;
} __packed;

#endif
