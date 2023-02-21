/*
 *  @description: Graphics output protocol interface.
 *  @author: Ian Marco Moffett.
 */

#include <dev/gop.h>
#include <printf.h>
#include <string.h>

static EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
static UINT32* framebuffer_addr = NULL;
static UINT32* backbuffer_addr = NULL;    /* TODO: Free when loading kernel */
static UINT32 pitch = 0;

UINT32 gop_get_index(UINT32 x, UINT32 y)
{
  return x + y * (pitch/4);
}

UINT32* gop_get_addr(void)
{
  return backbuffer_addr;
}

void gop_free_backbuffer(void)
{
  uefi_call_wrapper(BS->FreePool, 1, backbuffer_addr);
  backbuffer_addr = framebuffer_addr;
}

void gop_swap_buffers(void)
{
  for (UINT32 i = 0; i < gop_get_height()*pitch; ++i)
  {
    framebuffer_addr[i] = backbuffer_addr[i];
  }
}

UINT32 gop_get_width(void)
{
  return gop->Mode->Info->HorizontalResolution;
}

UINT32 gop_get_pitch(void)
{
  return pitch;
}

UINT32 gop_get_height(void)
{
  return gop->Mode->Info->VerticalResolution;
}

void gop_init(void)
{
  EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_STATUS s;

  s = uefi_call_wrapper(BS->LocateProtocol, 3, &gop_guid, NULL, (void**)&gop);
  if (EFI_ERROR(s))
  {
    printf("Unable to locate GOP.\n");
    halt();
  }

  framebuffer_addr = (UINT32*)gop->Mode->FrameBufferBase;
  pitch = 4*gop->Mode->Info->PixelsPerScanLine;

  /* Allocate memory for the backbuffer */
  uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData,
                    gop_get_height()*pitch,
                    (void**)&backbuffer_addr);

  memset(backbuffer_addr, 0, gop_get_height()*pitch);
}
