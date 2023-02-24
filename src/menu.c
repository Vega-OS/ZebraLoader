/*
 *  @description: ZebraLoader UI.
 *  @author: Ian Marco Moffett.
 */

#include <menu.h>
#include <bmp.h>
#include <dev/disk.h>
#include <dev/gop.h>

/*
 *  Returns a pointer to the
 *  wallpaper BMP.
 */

static void *get_wallpaper(void)
{
  EFI_FILE *bmp = disk_get_file(WALLPAPER_PATH);
  UINTN bmp_size = 0;
  EFI_STATUS status; 
  void *ret = NULL;

  if (bmp == NULL)
  {
    return NULL;
  }

  status = disk_get_size(bmp, &bmp_size);

  if (EFI_ERROR(status))
  {
    return NULL;
  }
  
  ret = AllocatePool(bmp_size);
  if (EFI_ERROR(status))
  {
    return NULL;
  }
  
  status = uefi_call_wrapper(bmp->Read, 3, bmp, &bmp_size, ret);

  if (EFI_ERROR(status))
  {
    return NULL;
  }

  return ret;
}

/*
 *  Draws wallpaper.
 */

static void draw_wallpaper(UINT32 start_x, UINT32 start_y,
                            UINT32 end_x, UINT32 end_y)
{
  void *bmp = get_wallpaper();

  if (bmp == NULL)
  {
    // TODO: Handle this. 
    return;
  }
  
  struct bmp_header* hdr = bmp; 
  if ((hdr->signature & 0xFF) != 'B' || (hdr->signature >> 8) != 'M')
  { 
    // TODO: Check other hdr stuff.
    return;
  }  

  UINT8 *image = (UINT8 *)((UINTN)bmp + hdr->data_offset);
  UINT32 *fb = gop_get_addr();
  UINT32 i = 0;

  for (int y = start_y; y < end_y; ++y)
  {
    for (int x = start_x; x < end_x; ++x)
    {
      /* These 2 lines scale the image to the size of the framebuffer */
      UINT32 bx = (x * hdr->width) / gop_get_width();
      UINT32 by = (y * hdr->height) / gop_get_height();

      UINT32 b = image[(by * hdr->width + bx) * 3];
      UINT32 g = image[(by * hdr->width + bx) * 3 + 1];
      UINT32 r = image[(by * hdr->width + bx) * 3 + 2];

      UINT32 rgb = ((r << 16) | (g << 8) | b);
      fb[gop_get_index(x, gop_get_height()-y)] = rgb;
    }
  }
}

void menu_start(void)
{   
  draw_wallpaper(0, 0, gop_get_width(), gop_get_height());
  gop_swap_buffers_at(0, 0, gop_get_width(), gop_get_height());

  for (;;)
  {
    __asm("pause");
  }
}
