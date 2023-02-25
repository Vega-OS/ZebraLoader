/*
 * elf file loader
 */
#include <dev/disk.h>
#include <elf.h>
#include <loader.h>

void load_kernel(CHAR16 *file_name)
{
  // Get file
  EFI_FILE *elf_file = disk_get_file(file_name);
  if (!elf_file)
  {
    Print(L"Could not get file %s\n", file_name);
    return;
  }

  // Get file size
  UINTN size;
  EFI_STATUS status = disk_get_size(elf_file, &size);
  if (EFI_ERROR(status))
  {
    Print(L"Could not get size of file %s\n", file_name);
    return;
  }

  // Allocate memory for file
  VOID *file_buffer;
  status = uefi_call_wrapper(BS->AllocatePool, 3,
                             EfiLoaderData,
                             size,
                             &file_buffer
  );

  if (EFI_ERROR(status))
  {
    Print(L"Could not allocate memory for file %s\n", file_name);
    return;
  }

  // Load file to memory
  status = uefi_call_wrapper(elf_file->Read, 3, 
                             elf_file,
                             &size,
                             file_buffer
  );

  if (EFI_ERROR(status))
  {
    Print(L"Could not load file %s to memory\n", file_name);
    return;
  }

  // Parse elf header
  Elf64_Ehdr *elf_hdr = (Elf64_Ehdr *)file_buffer;

  Print(L"Found ELF entry point: 0x%x\n", elf_hdr->e_entry);
}

