/*
 *  @description: Bootloader UI.
 *  @author: Ian Marco Moffett.
 */

#include <menu.h>
#include <bmp.h>
#include <dev/disk.h>
#include <dev/gop.h>

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

void menu_init(void)
{
  draw_background();
}
