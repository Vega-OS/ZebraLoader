/*
 *  @description: disk.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _DEV_DISK_H_
#define _DEV_DISK_H_

#include <def.h>

/*
 *  To be called once in
 *  efi_main()
 */

void disk_init(EFI_HANDLE image_handle);

/*
 *  Returns a file handle
 *  from a path.
 */

EFI_FILE *disk_get_file(CHAR16 *path);

/*
 *  Gets the file size of a file.
 *
 *  @file: File handle.
 *  @size: Pointer to a variable that will hold the size.
 */

EFI_STATUS disk_get_size(EFI_FILE *file, UINTN *size);

#endif
