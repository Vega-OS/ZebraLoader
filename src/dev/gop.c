/*
 *  @description: Graphics output protocol.
 *  @author: Queso Fuego.
 */

#include <dev/gop.h>

static UINT32 *backbuffer = NULL;
static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;

/*
 * Find GOP, get the current mode info
 * Mostly taken from https://wiki.osdev.org/GOP
 */
void gop_init(void)
{
  
  EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_STATUS status;

  // Verify GOP exists
  status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void **)&gop);
  if (EFI_ERROR(status))
  {
    Print(L"Unable to locate GOP");
  }


  // Get current GOP mode info
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
  UINTN SizeOfInfo, numModes, nativeMode;

  status = uefi_call_wrapper(gop->QueryMode, 
                             4, 
                             gop, 
                             (gop->Mode == NULL) ? 0 : gop->Mode->Mode, 
                             &SizeOfInfo,
                             &info
  );

  // Set the current mode
  if (status == EFI_NOT_STARTED) 
  {
    uefi_call_wrapper(gop->SetMode, 2, gop, 0);
  }

  if (EFI_ERROR(status))
  {
    Print(L"Unable to get native mode");
  }
  else
  {
    nativeMode = gop->Mode->Mode;
    numModes = gop->Mode->MaxMode;
  }

  // TEMP
  Print(L"Successfully set native GOP mode");
  backbuffer = AllocatePool(gop_get_size());
}

/*
 *  Returns the size of the framebuffer
 *  in bytes.
 */

UINT32 gop_get_size(void)
{
  return gop_get_pitch()*gop_get_height();
}

/*
 *  Returns the index into the
 *  framebuffer.
 */

UINT32 gop_get_index(UINT32 x, UINT32 y)
{
  return x+y*(gop_get_pitch()/4);
}

/*
 *  Returns the width of the framebuffer.
 */

UINT32 gop_get_width(void)
{
  return gop->Mode->Info->HorizontalResolution;
}

/*
 *  Returns the height of the framebuffer.
 */

UINT32 gop_get_height(void)
{
  return gop->Mode->Info->VerticalResolution;
}

/*
 *  Returns the address of the backbuffer.
 */

UINT32 *gop_get_addr(void)
{
  return backbuffer;
}

/*
 *  Returns framebuffer pitch.
 */

UINT32 gop_get_pitch(void)
{
  return 4*gop->Mode->Info->PixelsPerScanLine;
}

/*
 *  Writes the backbuffer from
 *  postion (start_x,start_y) to
 *  the main buffer until (end_x, end_y)
 */

void gop_swap_buffers_at(UINT32 start_x, UINT32 start_y,
                         UINT32 end_x, UINT32 end_y);
