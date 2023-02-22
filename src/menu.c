/*
 *  @description: Bootloader UI.
 *  @author: Ian Marco Moffett.
 */

#include <menu.h>
#include <bmp.h>
#include <dev/disk.h>
#include <dev/gop.h>

#define BLEND_GET_ALPHA(color) ((color >> 24) & 0x000000FF)
#define BLEND_GET_RED(color)   ((color >> 16)   & 0x000000FF)
#define BLEND_GET_GREEN(color) ((color >> 8)  & 0x000000FF)
#define BLEND_GET_BLUE(color)  ((color >> 0)   & 0X000000FF)

#define MENU_HEIGHT 200
#define MENU_WIDTH 400

/*
 *  Returns a pointer to BMP
 *  file data.
 */

static void* get_background_bmp(void)
{
  EFI_FILE* image = disk_get_file(L"wallpaper.bmp");
  UINTN image_size = 0;
  EFI_STATUS status;
  void* ret = NULL;

  if (image == NULL)
  {
    return NULL;
  }

  status = disk_get_file_size(image, &image_size);

  if (EFI_ERROR(status))
  {
    return NULL;
  }


  status = uefi_call_wrapper(BS->AllocatePool, 3,
                             EfiLoaderData,
                             image_size,
                             (void**)&ret
  );

  if (EFI_ERROR(status))
  {
    return NULL;
  }

  status = uefi_call_wrapper(image->Read, 3, image, &image_size, ret);
  
  if (EFI_ERROR(status))
  {
    Print(L"Failed to read BMP\n");
    uefi_call_wrapper(BS->FreePool, 1, ret);
    return NULL;
  }

  return ret;
}

/*
 *  Blends `color` with a black pixel.
 */

UINT32 blend_black(UINT32 color)
{
    UINT32 alpha1 = BLEND_GET_BLUE(color);
    UINT32 red1 = BLEND_GET_RED(color);
    UINT32 green1 = BLEND_GET_GREEN(color);
    UINT32 blue1 = BLEND_GET_BLUE(color);

    UINT32 alpha2 = BLEND_GET_ALPHA(0x000000);
    UINT32 red2 = BLEND_GET_RED(0x000000);
    UINT32 green2 = BLEND_GET_GREEN(0x00000);
    UINT32 blue2 = BLEND_GET_BLUE(0x00000);

    const float BLEND_AMT = 0.2;

    UINT32 r = (UINT32)((alpha1 * BLEND_AMT / 255) * red1);
    UINT32 g = (UINT32)((alpha1 * BLEND_AMT / 255) * green1);
    UINT32 b = (UINT32)((alpha1 * BLEND_AMT / 255) * blue1);

    r += (((255 - alpha1) * BLEND_AMT / 255) * (alpha2 * BLEND_AMT / 255)) * red2;
    g += (((255 - alpha1) * BLEND_AMT / 255) * (alpha2 * BLEND_AMT / 255)) * green2;
    b += (((255 - alpha1) * BLEND_AMT / 255) * (alpha2 * BLEND_AMT / 255)) * blue2;

    UINT32 new_alpha = (UINT32)(alpha1 + ((255 - alpha1) * BLEND_AMT / 255) * alpha2);
    UINT32 blend_res = (new_alpha << 24) |  (r << 16) | (g << 8) | (b << 0);
    return blend_res;
}

static void draw_background(void)
{
  void* bmp = get_background_bmp();

  if (bmp == NULL)
  {
    return;
  }

  struct bmp_header* header = bmp;
  
  if ((header->signature & 0xFF) != 'B' || (header->signature >> 8) != 'M')
  {
    return;
  }

  UINT8* image = (UINT8*)((UINTN)bmp + header->data_offset);
  UINT32* fb = gop_get_addr(); 
  UINT32 i = 0; 
  UINT32 start_x = 0;

  if (gop_get_width() > header->width)
  {
    start_x = (gop_get_width() - header->width) / 2;
  }

  for (int y = 0; y < gop_get_height(); ++y)
  { 
    for (int x = 0; x < gop_get_width(); ++x)
    {
      UINT32 bx = x % header->width;
      UINT32 by = y % header->height;
      UINT32 b = image[(by * header->width + bx) * 3];
      UINT32 g = image[(by * header->width + bx) * 3 + 1];
      UINT32 r = image[(by * header->width + bx) * 3 + 2];

      UINT32 rgb = ((r << 16) | (g << 8) | b);
      fb[gop_get_index(x, gop_get_height()-y)] = rgb;
    }
  }

  gop_swap_buffers();
}

static void draw_menu(void)
{
  UINT32 fb_height = gop_get_height();
  UINT32 fb_width = gop_get_width();

  UINT32 menu_start_x = (fb_width - MENU_WIDTH) / 2;
  UINT32 menu_start_y = (fb_height - MENU_HEIGHT) / 2;

  UINT32* fb = gop_get_addr();

  for (UINTN y = menu_start_y; y < menu_start_y+MENU_HEIGHT; ++y)
  {
    for (UINTN x = menu_start_x; x < menu_start_x+MENU_WIDTH; ++x)
    {
      UINT32 old_pixel = fb[gop_get_index(x, y)];
      fb[gop_get_index(x, y)] = blend_black(old_pixel);
    }
  }

  gop_swap_buffers();
}

void menu_init(void)
{
  draw_background();
  draw_menu();
}
