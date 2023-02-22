
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
  EFI_FILE_INFO* file_info;
  UINTN buffer_size = sizeof(EFI_FILE_INFO) + 1024;
  EFI_STATUS status;

  file_info = AllocatePool(buffer_size);

  if (file_info == NULL)
  {
    return EFI_OUT_OF_RESOURCES;
  }

  status = uefi_call_wrapper(file->GetInfo, 4,
                             file,
                             &gEfiFileInfoGuid,
                             &buffer_size,
                             file_info
  );

  if (EFI_ERROR(status))
  {
    return status;
  }
  
  *size = file_info->FileSize;
  FreePool(file_info);
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
