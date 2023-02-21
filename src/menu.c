/*
 *  @description: Bootloader menu.
 *  @author: Ian Marco Moffett.
 */

#include <menu.h>
#include <font.h>
#include <string.h>
#include <dev/gop.h>

#define MENU_HEIGHT 230
#define MENU_WIDTH  400
#define MENU_BG 0x00008B
#define MENU_FG_SEL 0xF88379
#define MENU_FG     0x880808
#define MENU_HELP "UP/DOWN to navigate, ENTER to select"
#define MENU_TITLE "ZebraLoader by Ian Moffett"
#define MENU_HELP_COLOR 0x87CEEB
#define MENU_TITLE_COLOR 0xF9F6EE
#define MENU_LINE_COLOR  0x71797E

static UINT32* fb = NULL;

static struct Font 
{
  UINT32 width;
  UINT32 height;
  UINT16* data;
} font = {
  .width = FONT_WIDTH,
  .height = FONT_HEIGHT,
  .data = (UINT16*)DEFAULT_FONT_DATA
};

typedef enum
{
  MENU_BOOT,
  MENU_SHUTDOWN,
  MENU_TOP,         /* Max menu_entry_strtab entries */
} menu_entry_t;

static menu_entry_t current_entry = MENU_BOOT;
static const char* const menu_entry_strtab[] = {
  "Boot",
  "Shutdown"
};

static void putch(UINT32 x, UINT32 y, char c, UINT32 fg, UINT32 bg)
{
  c -= 32;
  for (UINT32 cx = 0; cx < font.width; ++cx) 
  {
    for (UINT32 cy = 0; cy < font.height; ++cy) 
    {
      UINT16 col = (font.data[(UINT64)c * font.width + cx] >> cy) & 1;
      fb[gop_get_index(x + cx, y + cy)] = col ? fg : bg;
    }
  }
}

static void putstr(const char* str, UINT32 x, UINT32 y,
                   UINT32 fg, UINT32 bg)
{
  for (UINTN i = 0; i < strlen(str); ++i)
  {
    putch(x, y, str[i], fg, bg);
    x += FONT_WIDTH;
  }
}

static uintptr_t get_text_x(const char* str, UINT32 menu_start_x)
{
  UINT32 text_width = strlen(str)*FONT_WIDTH;
  return menu_start_x+(MENU_WIDTH-text_width)/2;
}

static void draw_horizontal_line(UINT32 y, UINT32 menu_start_x)
{
  for (UINTN x = menu_start_x; x < menu_start_x+MENU_WIDTH; ++x)
  {
    fb[gop_get_index(x, y)] = MENU_LINE_COLOR;
  }
}

static void draw_vertical_line(UINT32 x, UINT32 menu_start_y)
{
  for (UINTN y = menu_start_y; y < menu_start_y+MENU_HEIGHT; ++y)
  {
    fb[gop_get_index(x, y)] = MENU_LINE_COLOR;
  }
}

static void draw_menu_box(menu_entry_t selected_entry)
{
  UINT32 fb_height = gop_get_height();
  UINT32 fb_width = gop_get_width(); 
  
  UINT32 menu_start_x = (fb_width - MENU_WIDTH) / 2;
  UINT32 menu_start_y = (fb_height - MENU_HEIGHT) / 2;

  for (UINT32 y = menu_start_y; y < menu_start_y+MENU_HEIGHT; ++y)
  {
    for (UINT32 x = menu_start_x; x < menu_start_x+MENU_WIDTH; ++x)
    {
      fb[gop_get_index(x, y)] = MENU_BG;
    }
  }

  /* Start x and y positions for each option */
  UINT32 option_start_y = menu_start_y+(MENU_HEIGHT-FONT_HEIGHT)/2;
  UINT32 option_start_x;

  UINT32 title_start_x = get_text_x(MENU_TITLE, menu_start_x);
  UINT32 title_start_y = menu_start_y+FONT_HEIGHT;
  putstr(MENU_TITLE, title_start_x, title_start_y, MENU_TITLE_COLOR, MENU_BG);
  
  
  /* Draw horizontal line */
  title_start_y += FONT_HEIGHT+4;
  draw_horizontal_line(title_start_y, menu_start_x);

  /* Draw vertical lines */
  draw_vertical_line(menu_start_x, menu_start_y);
  draw_vertical_line(menu_start_x + MENU_WIDTH, menu_start_y);
  
  /* Draw HELP menu */
  title_start_y += FONT_HEIGHT+5;
  title_start_x = get_text_x(MENU_HELP, menu_start_x);
  putstr(MENU_HELP, title_start_x, title_start_y, MENU_HELP_COLOR, MENU_BG); 

  for (UINTN i = 0; i < MENU_TOP; ++i)
  {
    option_start_x = get_text_x(menu_entry_strtab[i], menu_start_x);

    UINT32 fg = (selected_entry == i) ? MENU_FG_SEL : MENU_FG;
    putstr(menu_entry_strtab[i], option_start_x, option_start_y, fg, MENU_BG);

    option_start_y += FONT_HEIGHT;
  }

  gop_swap_buffers();
}

static void menu_move_up(void)
{
  if (current_entry > 0)
  {
    --current_entry;
    draw_menu_box(current_entry);
  }
}

static void menu_move_down(void)
{
  if (current_entry < MENU_TOP-1)
  {
    ++current_entry;
    draw_menu_box(current_entry);
  }
}


void menu_stop(void)
{
  memset(fb, 0, gop_get_pitch()*gop_get_height());
  gop_swap_buffers();
}

void menu_start(void)
{
  fb = gop_get_addr();
  draw_menu_box(MENU_BOOT);
  EFI_INPUT_KEY key;

  UINT8 is_looping = 1; 
  while (is_looping)
  {
    EFI_STATUS s = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, 
                                     ST->ConIn, &key);

    if (s == EFI_NOT_READY)
    {
      uefi_call_wrapper(BS->Stall, 1, 1000);
      continue;
    }
    
    /* NOTE: Enter is handled at the bottom of this loop */
    switch (key.ScanCode)
    {
      case SCAN_UP:
        menu_move_up();
        continue;
      case SCAN_DOWN:
        menu_move_down();
        continue;
    }
    
    /* Vim-like keys */
    if (key.UnicodeChar == L'j')
    {
      menu_move_down();
      continue;
    }
    else if (key.UnicodeChar == L'k')
    {
      menu_move_up();
      continue;
    }
  
    /* Assuming we pressed ENTER */
    if (key.UnicodeChar == '\r')
    {
      switch (current_entry)
      {
        case MENU_SHUTDOWN:
          uefi_call_wrapper(RT->ResetSystem, 4, EfiResetShutdown,
                            0, 0, NULL);

          __asm__ __volatile__("cli; hlt");
          break;
        case MENU_BOOT:
          is_looping = 0;
          break;
        default:
          break;
      }
    }
  }
    
  menu_stop();
}
