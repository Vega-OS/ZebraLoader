#include <dev/gop.h>

static EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;
static UINT32* backbuffer = NULL;

/*
 *  Returns GOP.
 */

static EFI_GRAPHICS_OUTPUT_PROTOCOL* get_gop(void)
{
  EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
  EFI_STATUS status;

  status = uefi_call_wrapper(BS->LocateProtocol, 3, &gop_guid, NULL, 
                             (void**)&gop);

  if (EFI_ERROR(status))
  {
    Print(L"ERROR: Graphics output protocol unavailable!\n");
    halt();
  }

  return gop;
}

UINT32 gop_get_size(void)
{
  return gop->Mode->FrameBufferSize;
}

UINT32 gop_get_width(void)
{
  return gop->Mode->Info->HorizontalResolution;
}

UINT32 gop_get_height(void)
{
  return gop->Mode->Info->VerticalResolution;
}

UINT32* gop_get_addr(void)
{
  return backbuffer;
}

void gop_swap_buffers(void)
{
  for (UINT32 i = 0; i < gop->Mode->FrameBufferSize/4; ++i)
  {
    ((UINT32*)gop->Mode->FrameBufferBase)[i] = backbuffer[i];
  }
}

void gop_init(void)
{
  EFI_STATUS status;

  Print(L"Detecting Graphics Output Protocol..\n");
  gop = get_gop();
  
  Print(L"Found GOP at address 0x%x, occupying %d pages.\n",
        gop->Mode->FrameBufferBase,
        gop->Mode->FrameBufferSize/4096
  );

  uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData,
                    gop->Mode->FrameBufferSize,
                    (void**)&backbuffer
  );
  
  /* Clear the backbuffer */
  for (UINT32 i = 0; i < gop->Mode->FrameBufferSize/4; ++i)
  {
    backbuffer[i] = 0;
  }
}
