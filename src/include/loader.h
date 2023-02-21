/*
 *  @description: loader.h
 *  @author: Ian Marco Moffett.
 */

#ifndef _LOADER_H_
#define _LOADER_H_

#include <def.h>

__attribute__((noreturn)) void load_kernel(EFI_HANDLE image_handle);

#endif
