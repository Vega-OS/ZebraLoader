/*
 *  @description: Disk interface.
 *  @author: Ian Marco Moffett.
 */

#include <dev/disk.h>

static EFI_HANDLE img_handle;


void disk_init(EFI_HANDLE image_handle)
{
  img_handle = image_handle;
}

EFI_FILE *disk_get_file(CHAR16 *path)
{
  EFI_FILE *file = NULL;
  EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
  EFI_GUID simple_fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_GUID loaded_img_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_STATUS s;

  s = uefi_call_wrapper(BS->HandleProtocol,
                        3,
                        img_handle,
                        &loaded_img_guid,
                        (void **)&loaded_image
  );


  if (EFI_ERROR(s))
  {
    return NULL;
  }

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs = NULL;
  s = uefi_call_wrapper(BS->HandleProtocol,
                        3,
                        loaded_image->DeviceHandle,
                        &simple_fs_guid,
                        (void **)&fs
  );

  if (EFI_ERROR(s))
  {
    return NULL;
  }

  EFI_FILE *dir = NULL;
  s = uefi_call_wrapper(fs->OpenVolume, 2, fs, &dir);

  if (EFI_ERROR(s))
  {
    return NULL;
  }

  s = uefi_call_wrapper(dir->Open,
                        5,
                        dir,
                        &file,
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

EFI_STATUS disk_get_size(EFI_FILE *file, UINTN *size)
{
  EFI_FILE_INFO *file_info = NULL;
  UINTN buffer_size = sizeof(EFI_FILE_INFO) + 1024;
  EFI_STATUS s;

  file_info = AllocatePool(buffer_size);
  if (file_info == NULL)
  {
    return EFI_OUT_OF_RESOURCES;
  }

  s = uefi_call_wrapper(file->GetInfo,
                        4,
                        file,
                        &gEfiFileInfoGuid,
                        &buffer_size,
                        file_info
  );

  if (EFI_ERROR(s))
  {
    return s;
  }

  *size = file_info->FileSize;
  FreePool(file_info);
  return EFI_SUCCESS;
}
