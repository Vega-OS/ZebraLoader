/*
 *  @description: Bootloader UI.
 *  @author: Ian Marco Moffett.
 */

#include <menu.h>
#include <bmp.h>
#include <font.h>
#include <string.h>
#include <dev/disk.h>
#include <dev/gop.h>

#define BLEND_GET_ALPHA(color) ((color >> 24) & 0x000000FF)
#define BLEND_GET_RED(color)   ((color >> 16)   & 0x000000FF)
#define BLEND_GET_GREEN(color) ((color >> 8)  & 0x000000FF)
#define BLEND_GET_BLUE(color)  ((color >> 0)   & 0X000000FF)

#define MENU_HEIGHT 325
#define MENU_WIDTH  400
#define MENU_TITLE "ZebraLoader by Ian Moffett"
#define MENU_HELP  "UP/DOWN to navigate, RIGHT to select"
#define MENU_ENTRY_SEL_COLOR 0x0096FF
#define MENU_ENTRY_COLOR 0xA9A9A9
#define MENU_TITLE_COLOR 0xFCF5E5
#define MENU_LINE_COLOR 0x71797E

typedef enum
{
  MENU_BOOT,
  MENU_REBOOT,
  MENU_SHUTDOWN,
  MENU_TOP,       /* Max menu entry + 1 */
} menu_entry_t;

static const char* menu_entry_strtab[MENU_TOP] = {
  "Boot",
  "Reboot",
  "Shutdown"
};

static menu_entry_t selected_entry = MENU_BOOT;

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
 *  Places a character on the screen at (x,y).
 *  
 *  @x: X position.
 *  @y: Y position.
 *  @fg: Foreground.
 *  @bg: Background.
 */

static void putch(UINT32 x, UINT32 y, char c, UINT32 fg)
{
  UINT32* fb_addr = gop_get_addr();

  c -= 32;
  for (UINT32 cx = 0; cx < FONT_WIDTH; ++cx) 
  {
    for (UINT32 cy = 0; cy < FONT_HEIGHT; ++cy) 
    {
      UINT16 col = (DEFAULT_FONT_DATA[(UINTN)c * FONT_WIDTH + cx] >> cy) & 1;

      if (col)
      {
        fb_addr[gop_get_index(x + cx, y + cy)] = fg;
      }
    }
  }
}

/*
 *  Does the same thing as putch() but
 *  with strings.
 */

static void putstr(UINT32 x, UINT32 y, const char* str, UINT32 fg)
{
  for (UINTN i = 0; i < strlen(str); ++i)
  {
    putch(x, y, str[i], fg);
    x += FONT_WIDTH;
  }
}

/*
 *  Blends `color` with a black pixel.
 */

static UINT32 blend_black(UINT32 color)
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

/*
 *  Gets the x position of a string
 *  so it's centered in the menu.
 */

static UINT32 get_str_x(const char* str, UINT32 menu_start_x)
{
  UINTN text_width = strlen(str) * FONT_WIDTH;
  return menu_start_x + (MENU_WIDTH - text_width) / 2;
}

/*
 *  Draws a horizontal line
 *  on the menu.
 */

static UINT32 draw_horizontal_line(UINT32 menu_start_x, UINT32 y)
{
  UINT32* fb = gop_get_addr();

  for (UINTN x = menu_start_x; x < menu_start_x+MENU_WIDTH; ++x)
  {
    fb[gop_get_index(x, y)] = MENU_LINE_COLOR;
  }
}


/*
 *  Draws a vertical line
 *  on the menu.
 */

static UINT32 draw_vertical_line(UINT32 x, UINT32 menu_start_y)
{
  UINT32* fb = gop_get_addr();

  for (UINTN y = menu_start_y; y < menu_start_y+MENU_HEIGHT; ++y)
  {
    fb[gop_get_index(x, y)] = MENU_LINE_COLOR;
  }
}

/*
 *  @start_x: Start x position to draw image to.
 *  @start_y: Start y position to draw the image to.
 *  @end_x: X position to stop drawing image at.
 *  @end_y: Y position to stop drawing image at.
 */

static void draw_background(UINT32 start_x, UINT32 start_y,
                            UINT32 end_x, UINT32 end_y)
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

  for (int y = start_y; y < end_y; ++y)
  { 
    for (int x = start_x; x < end_x; ++x)
    {
      /* These 2 lines scale the image to the size of the framebuffer */
      UINT32 bx = (x * header->width) / gop_get_width();
      UINT32 by = (y * header->height) / gop_get_height();

      UINT32 b = image[(by * header->width + bx) * 3];
      UINT32 g = image[(by * header->width + bx) * 3 + 1];
      UINT32 r = image[(by * header->width + bx) * 3 + 2];

      UINT32 rgb = ((r << 16) | (g << 8) | b);
      fb[gop_get_index(x, gop_get_height()-y)] = rgb;
    }
  }
}

static UINT32 get_menu_start_x(void)
{
  UINT32 fb_width = gop_get_width();
  return (fb_width - MENU_WIDTH) / 2;
}

static UINT32 get_menu_start_y(void)
{
  UINT32 fb_height = gop_get_height();
  return (fb_height - MENU_HEIGHT) / 2;
}

static void draw_menu(menu_entry_t selected_entry)
{
  UINT32 fb_height = gop_get_height();
  UINT32 fb_width = gop_get_width();

  UINT32 menu_start_x = get_menu_start_x();
  UINT32 menu_start_y = get_menu_start_y();

  UINT32* fb = gop_get_addr();

  for (UINTN y = menu_start_y; y < menu_start_y+MENU_HEIGHT; ++y)
  {
    for (UINTN x = menu_start_x; x < menu_start_x+MENU_WIDTH; ++x)
    {
      UINT32 old_pixel = fb[gop_get_index(x, y)];
      fb[gop_get_index(x, y)] = blend_black(old_pixel);
    }
  }
  
  UINTN title_y = menu_start_y+5;
  putstr(get_str_x(MENU_TITLE, menu_start_x), title_y,
         MENU_TITLE,
         MENU_TITLE_COLOR
  );
  
  title_y += FONT_HEIGHT;
  putstr(get_str_x(MENU_HELP, menu_start_x), title_y,
         MENU_HELP,
         MENU_TITLE_COLOR
  );
  
  /*
   * @y: Initial y for the menu entry.
   *
   *     FONT_HEIGHT*2 added to title_y
   *     so we don't write over the title.
   */
  UINT32 y = title_y+(FONT_HEIGHT*2);
  for (UINTN i = 0; i < MENU_TOP; ++i)
  {
    UINT32 color = selected_entry == i ? MENU_ENTRY_SEL_COLOR
                                       : MENU_ENTRY_COLOR;

    putstr(get_str_x(menu_entry_strtab[i], menu_start_x), y,
           menu_entry_strtab[i],
           color
    );

    y += FONT_HEIGHT;
  }

  draw_horizontal_line(menu_start_x, title_y+FONT_HEIGHT);
  draw_vertical_line(menu_start_x, menu_start_y);
  draw_vertical_line(menu_start_x + (MENU_WIDTH-1), menu_start_y);
}

static void refresh(void)
{
  UINT32 menu_x = get_menu_start_x();
  UINT32 menu_y = get_menu_start_y();
  draw_background(menu_x, menu_y,
                  menu_x + MENU_WIDTH,
                  menu_y + MENU_HEIGHT
  );

  draw_menu(selected_entry);
  gop_swap_buffers();
}

static void move_up(void)
{
  if (selected_entry > 0)
  {
    --selected_entry;
  }
  
  refresh();
}

static void move_down(void)
{
  if (selected_entry < MENU_TOP-1)
  {
    ++selected_entry;
  }

  refresh();
}

static void select_entry(void)
{
  switch (selected_entry)
  {
    case MENU_REBOOT:
      uefi_call_wrapper(RT->ResetSystem, 4,
                        EfiResetWarm,           /* Soft reboot */
                        0,
                        0,
                        NULL
      );
      
      for (;;);
    case MENU_SHUTDOWN:
      uefi_call_wrapper(RT->ResetSystem, 4,
                        EfiResetShutdown,
                        0,
                        0,
                        NULL
      );

      for (;;);
  }
}

void menu_init(void)
{
  EFI_INPUT_KEY key;
  EFI_STATUS status;
  UINT8 is_looping = 1;

  draw_background(0, 0, gop_get_width(), gop_get_height());
  draw_menu(MENU_BOOT);
  gop_swap_buffers();

  while (is_looping)
  {
    status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &key);

    if (status == EFI_NOT_READY)
    {
      uefi_call_wrapper(BS->Stall, 1, 1000);
      continue;
    }

    switch (key.ScanCode)
    {
      case SCAN_UP:
        move_up();
        break;
      case SCAN_DOWN:
        move_down();
        break;
      case SCAN_RIGHT:
        select_entry();
        break;
    }

    if (key.UnicodeChar == L'j')
    {
      move_down();
    }
    else if (key.UnicodeChar == L'k')
    {
      move_up();
    }
  }
}
