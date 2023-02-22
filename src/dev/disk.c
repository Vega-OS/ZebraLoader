
/*
 *  @description: Disk interface.
 *  @author: Ian Marco Moffett.
 */

#include <dev/disk.h>

static EFI_FILE_HANDLE volume;
static EFI_HANDLE image_handle;

EFI_FILE* disk_get_file(CHAR16* path)
{
  EFI_FILE* file;
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
  EFI_GUID simple_fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_GUID loaded_img_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_STATUS s;
  
  s = uefi_call_wrapper(BS->HandleProtocol, 3,
                        image_handle, &loaded_img_guid,
                        (void**)&loaded_image
  );

  if (EFI_ERROR(s))
  {

    return NULL;
  }

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs = NULL;
  uefi_call_wrapper(BS->HandleProtocol, 3,
                    loaded_image->DeviceHandle,
                    &simple_fs_guid,
                    (void**)&fs
  );
  
  EFI_FILE* dir = NULL;
  s = uefi_call_wrapper(fs->OpenVolume, 2, fs, &dir);

  if (EFI_ERROR(s))
  {
    return NULL;
  }

  s = uefi_call_wrapper(dir->Open, 5, 
                        dir, &file,
                        path,
                        EFI_FILE_MODE_READ,
                        EFI_FILE_READ_ONLY
  );

  if (EFI_ERROR(s))
  {
    return NULL;
  }
  
  return file;
}

EFI_STATUS disk_get_file_size(EFI_FILE* file, UINTN* size)
{
  EFI_STATUS status;
  EFI_FILE_INFO* file_info;
  UINTN buffer_size = 0;

  /* Get the buffer size for file info */
  uefi_call_wrapper(file->GetInfo, 4,
                    file,
                    &gEfiFileInfoGuid,
                    &buffer_size,
                    NULL

  );

  if (status != EFI_BUFFER_TOO_SMALL)
  {
    return status;
  }
  
  status = uefi_call_wrapper(BS->AllocatePool, 3,
                             EfiLoaderData,
                             buffer_size,
                             (void**)&file_info
  );

  if (EFI_ERROR(status))
  {
    return status;
  }

  /* Get the file info */
  status = uefi_call_wrapper(file->GetInfo, 4,
                             &gEfiFileInfoGuid,
                             &buffer_size,
                             (void*)file_info
  );

  *size = file_info->FileSize;
  uefi_call_wrapper(BS->FreePool, 1, file_info);
  return EFI_SUCCESS;
}

void disk_init(EFI_HANDLE image)
{
  image_handle = image;
  EFI_LOADED_IMAGE* loaded_image = NULL;
  EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_FILE_IO_INTERFACE* io_volume;
  EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

  uefi_call_wrapper(BS->HandleProtocol, 3, image, &lip_guid,
                    (void**)&loaded_image
  );

  uefi_call_wrapper(BS->HandleProtocol, 3,
                    loaded_image->DeviceHandle,
                    &fs_guid,
                    (void*)&io_volume
  );

  uefi_call_wrapper(io_volume->OpenVolume, 2, io_volume, &volume);
}
