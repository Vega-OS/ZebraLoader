/*
 * @description: ELF file loader
 * @author: Queso Fuego
 */

#include <dev/disk.h>
#include <elf.h>
#include <loader.h>

/*
 *  Returns 1 if the ELF header
 *  passed is valid.
 */

static UINT8 is_eh_valid(Elf64_Ehdr *eh)
{
  return eh->e_ident[EI_CLASS] == ELFCLASS64
         && eh->e_ident[EI_DATA] == ELFDATA2LSB
         && eh->e_type == ET_EXEC
         && eh->e_machine == EM_X86_64;
}

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

  if (!is_eh_valid(elf_hdr))
  {
    Print(L"Kernel ELF header not valid!\n");
    return;
  }

  Print(L"Found ELF entry point: 0x%x\n", elf_hdr->e_entry);
}

