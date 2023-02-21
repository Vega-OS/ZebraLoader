/*
 *  @description: disk.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _DEV_DISK_H_
#define _DEV_DISK_H_

#include <def.h>

void disk_init(EFI_HANDLE image);
EFI_FILE* disk_get_file(CHAR16* path);

#endif
