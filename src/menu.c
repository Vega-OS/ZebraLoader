/*
 *  @description: ZebraLoader UI.
 *  @author: Ian Marco Moffett.
 */

#include <menu.h>
#include <bmp.h>
#include <font.h>
#include <string.h>
#include <dev/disk.h>
#include <dev/gop.h>
#include <loader.h>

#define MENU_HEIGHT 400
#define MENU_WIDTH  500
#define MENU_TEXT_COLOR 0xF9F6EE
#define MENU_LINE_COLOR 0x71797E
#define MENU_LINE_THICKNESS 2
#define MENU_TITLE "ZebraLoader v0.0.4"     /* NOTE: Update version here too. */
#define MENU_HELP  "Use ARROWS to navigate and ENTER to select"

#define MENU_ENTRY_SEL_FG 0x1B1212
#define MENU_ENTRY_SEL_BG 0xA9A9A9
#define MENU_ENTRY_COLOR 0xA9A9A9
#define MENU_LINE_SPACING 40

#if MENU_LINE_THICKNESS == 0
#error "MENU_LINE_THICKNESS == 0"
#endif

#if MENU_WIDTH == 0
#error "MENU_WIDTH == 0"
#endif

#if MENU_HEIGHT == 0
#error "MENU_HEIGHT == 0"
#endif

typedef enum
{
  MENU_ENTRY_BOOT,
  MENU_ENTRY_SHUTDOWN,
  MENU_ENTRY_REBOOT,
  MENU_ENTRY_TOP,
} menu_entry_t;


static const char* menu_entry_strtab[] = {
  BOOT_ENTRYNAME,
  "Shutdown",
  "Reboot",
};

static menu_entry_t selected_entry = MENU_ENTRY_BOOT;

#define BLEND_GET_ALPHA(color) ((color >> 24) & 0x000000FF)
#define BLEND_GET_RED(color)   ((color >> 16) & 0x000000FF)
#define BLEND_GET_GREEN(color) ((color >> 8)  & 0x000000FF)
#define BLEND_GET_BLUE(color)  ((color >> 0)  & 0X000000FF)


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

static UINT32 get_menu_start_x(void)
{
  UINT32 fb_width = gop_get_width();
  return SAFE_DIV(fb_width - MENU_WIDTH, 2);
}

static UINT32 get_menu_start_y(void)
{
  UINT32 fb_height = gop_get_height();
  return SAFE_DIV(fb_height - MENU_HEIGHT, 2);
}

/*
 *  Places a character on the screen at (x,y).
 *  
 *  @x: X position.
 *  @y: Y position.
 *  @fg: Foreground.
 *  @bg: Background color.
 *  @use_bg: 1 to use a background color.
 */

static void putch(UINT32 x, UINT32 y, char c, UINT32 fg, UINT32 bg,
                  UINT32 use_bg)
{
  UINT32 *fb_addr = gop_get_addr();

  c -= 32;
  for (UINT32 cx = 0; cx < FONT_WIDTH; ++cx) 
  {
    for (UINT32 cy = 0; cy < FONT_HEIGHT; ++cy) 
    {
      UINT16 col = (DEFAULT_FONT_DATA[(UINTN)c * FONT_WIDTH + cx] >> cy) & 1;

      if (use_bg)
      {
        fb_addr[gop_get_index(x + cx, y + cy)] = col ? fg : bg;
      }
      else if (col)
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

static void putstr(UINT32 x, UINT32 y, const char *str, UINT32 fg, UINT32 bg,
                   UINT32 use_bg)
{
  for (UINTN i = 0; i < strlen(str); ++i)
  {
    putch(x, y, str[i], fg, bg, use_bg);
    x += FONT_WIDTH;
  }
}

/*
 *  Gets the x position of a string
 *  so it's centered in the menu.
 */

static UINT32 get_str_x(const char *str)
{
  UINT32 menu_start_x = get_menu_start_x();
  UINTN text_width = strlen(str) * FONT_WIDTH;
  return menu_start_x + (MENU_WIDTH - text_width) / 2;
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
    /* TODO: Handle this.  */
    return;
  }
  
  struct bmp_header* hdr = bmp; 
  if ((hdr->signature & 0xFF) != 'B' || (hdr->signature >> 8) != 'M')
  { 
    /* TODO: Check other hdr stuff. */
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
      UINT32 bx = SAFE_DIV(x * hdr->width, gop_get_width());
      UINT32 by = SAFE_DIV(y * hdr->height, gop_get_height());

      UINT32 b = image[(by * hdr->width + bx) * 3];
      UINT32 g = image[(by * hdr->width + bx) * 3 + 1];
      UINT32 r = image[(by * hdr->width + bx) * 3 + 2];

      UINT32 rgb = ((r << 16) | (g << 8) | b);      
      fb[gop_get_index(x, gop_get_height()-y)] = rgb;
    }
  }
}

static UINT32 blend_transparent(UINT32 old_pixel, UINT32 new_pixel, float alpha)
{
  UINT32 blended_pixel;
  float old_alpha = 1.0f - alpha;

  UINT32 old_r = (old_pixel >> 16) & 0xFF;
  UINT32 old_g = (old_pixel >> 8) & 0xFF;
  UINT32 old_b = old_pixel & 0xFF;

  UINT32 new_r = (new_pixel >> 16) & 0xFF;
  UINT32 new_g = (new_pixel >> 8) & 0xFF;
  UINT32 new_b = new_pixel & 0xFF;

  UINT32 r = (UINT32)(old_r * old_alpha + new_r * alpha);
  UINT32 g = (UINT32)(old_g * old_alpha + new_g * alpha);
  UINT32 b = (UINT32)(old_b * old_alpha + new_b * alpha);

  blended_pixel = (r << 16) | (g << 8) | b;
  return blended_pixel;
}

/*
 *  Draws a line on the menu like this:
 *
 *  -------------------------------------
 */

static void draw_line_x(UINT32 y_start, UINT32 x_start, UINT32 x_end)
{
  UINT32 *fb = gop_get_addr();
  
  for (UINT32 x = x_start; x < x_end; ++x)
  {
    fb[gop_get_index(x, y_start)] = MENU_LINE_COLOR;
  }
}

/*
 *  Draws a line on the menu like this:
 *
 *              |
 *              |
 *              |
 *              |
 */

static void draw_line_y(UINT32 x_start, UINT32 y_start, UINT32 y_end)
{
  UINT32 *fb = gop_get_addr();
  
  for (UINT32 y = y_start; y < y_end; ++y)
  {
    fb[gop_get_index(x_start, y)] = MENU_LINE_COLOR;
  }
    
  ++x_start;
}

static void draw_menu_lines(const UINT32 Y_OFF)
{
  UINT32 menu_start_x = get_menu_start_x();
  UINT32 menu_start_y = get_menu_start_y();

  const UINT32 X_OFF = Y_OFF;

  /* Draw the top line. */
  draw_line_x(menu_start_y + Y_OFF,
              menu_start_x+X_OFF,
              (menu_start_x+MENU_WIDTH)-X_OFF
  );

  /* Draw the bottom line. */
  draw_line_x((menu_start_y + MENU_HEIGHT) - Y_OFF,
              menu_start_x + X_OFF,
              (menu_start_x + MENU_WIDTH) - X_OFF
  );

  /* Draw the left line. */
  draw_line_y(menu_start_x + X_OFF,
              menu_start_y + Y_OFF,
              (menu_start_y+MENU_HEIGHT) - Y_OFF
  );

  /* Draw the right line. */
  draw_line_y((menu_start_x + MENU_WIDTH) - X_OFF,
              menu_start_y + Y_OFF,
              (menu_start_y + MENU_HEIGHT) - Y_OFF
  );
}

static void draw_menu(void)
{
  /* Get the starting position to draw the menu square.  */
  UINT32 menu_start_x = get_menu_start_x();
  UINT32 menu_start_y = get_menu_start_y();

  /* Get the framebuffer address to draw on. */
  UINT32 *fb = gop_get_addr();

  for (UINT32 x = menu_start_x; x < menu_start_x+MENU_WIDTH; ++x)
  {
    for (UINT32 y = menu_start_y; y < menu_start_y+MENU_HEIGHT; ++y)
    {
      UINT32 old_pixel = fb[gop_get_index(x, y)];
      fb[gop_get_index(x, y)] = blend_transparent(old_pixel, 0x000000, 0.9);
    }
  }
  
  draw_menu_lines(MENU_LINE_SPACING);
  UINT32 current_entry_y = menu_start_y+FONT_HEIGHT;
  putstr(get_str_x(MENU_TITLE),
         current_entry_y,
         MENU_TITLE,
         MENU_TEXT_COLOR,
         0,
         0);

  current_entry_y += FONT_HEIGHT*2;
  
  putstr(get_str_x(MENU_HELP),
         menu_start_y+(MENU_HEIGHT-20),
         MENU_HELP,
         MENU_TEXT_COLOR,
         0,
         0
  );

  for (menu_entry_t i = 0; i < MENU_ENTRY_TOP; ++i)
  {
    UINT32 fg = i == selected_entry ? MENU_ENTRY_SEL_FG : MENU_ENTRY_COLOR;

    /* Entry position. */
    UINT32 ypos = (menu_start_y+MENU_LINE_SPACING)+FONT_HEIGHT*i;
    UINT32 xpos = get_str_x(menu_entry_strtab[i]);

    ypos += FONT_HEIGHT;

    putstr(xpos,
           ypos,
           menu_entry_strtab[i],
           fg,
           MENU_ENTRY_SEL_BG,
           i == selected_entry
    );

    current_entry_y += FONT_HEIGHT;
  }
}

static void draw_ui(void)
{
  draw_wallpaper(0, 0, gop_get_width(), gop_get_height());
  draw_menu();
  gop_swap_buffers_at(0, 0, gop_get_width(), gop_get_height());
}

static void next_menu_entry(void)
{
  if (selected_entry < MENU_ENTRY_TOP-1)
  {
    ++selected_entry;
  }
}

static void prev_menu_entry(void)
{
  if (selected_entry > 0)
  {
    --selected_entry;
  }
}

static void refresh_menu(void)
{
  draw_wallpaper(get_menu_start_x(),
                 get_menu_start_y(),
                 get_menu_start_x()+MENU_WIDTH,
                 get_menu_start_y()+MENU_HEIGHT
  );

  draw_menu();
  gop_swap_buffers_at(get_menu_start_x(),
                      get_menu_start_y(),
                      get_menu_start_x()+MENU_WIDTH,
                      get_menu_start_y()+MENU_HEIGHT
  );
}

/*
 *  Runs the selected menu entry.
 */

static void run_menu_entry(void)
{
  switch (selected_entry)
  {
    case MENU_ENTRY_REBOOT:
      uefi_call_wrapper(RT->ResetSystem,
                        4,
                        EfiResetCold,
                        0,
                        0,
                        NULL);

      for (;;);
    case MENU_ENTRY_SHUTDOWN:
      uefi_call_wrapper(RT->ResetSystem,
                        4,
                        EfiResetShutdown,
                        0,
                        0,
                        NULL);
      for (;;);
    case MENU_ENTRY_BOOT:
      clear_backbuffer();
      gop_swap_buffers();
      load_kernel(__KERNEL_ELF);
      for (;;);
  }
}

/*
 *  Handles a keypress in the
 *  menu main loop.
 */

static void handle_efi_input_key(EFI_INPUT_KEY key)
{
  switch (key.UnicodeChar)
  {
    case '+':
      gop_next_mode();
      draw_ui(); 
      return;
    case L'j':
      next_menu_entry();
      refresh_menu();
      return;
    case L'k':
      prev_menu_entry();
      refresh_menu();
      return;
    case L'\r':
      run_menu_entry();
      return;

  }

  switch (key.ScanCode)
  {
    case SCAN_DOWN:
      next_menu_entry();
      refresh_menu();
      return;
    case SCAN_UP:
      prev_menu_entry();
      refresh_menu();
      return;
  }
}

void menu_start(void)
{
  draw_ui();

  EFI_INPUT_KEY key;
  EFI_STATUS s;

  for (;;)
  {
    s = uefi_call_wrapper(ST->ConIn->ReadKeyStroke,
                          2,
                          ST->ConIn,
                          &key
    );
    
    if (s == EFI_NOT_READY)
    {
      /* 
       * Keystroke not ready so we stall for 1000 to prevent
       * overloading the system.
       */
      uefi_call_wrapper(BS->Stall, 1, 1000);
      continue;
    }
    
    handle_efi_input_key(key);
  }
}
