AC_INIT([zebra_loader], [0.0.1], [ian@ian-moffett.com])
CROSS_ERR_MSG="Please run tools/cross.sh"

AC_CHECK_FILE([cross/bin/x86_64-elf-gcc], [], AC_MSG_ERROR([$CROSS_ERR_MSG]))
AC_CHECK_FILE([cross/bin/x86_64-elf-ld], [], AC_MSG_ERROR([$CROSS_ERR_MSG]))

CFLAGS="-fpic -ffreestanding -fno-stack-protector -fno-stack-check  \\
				-fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c   \\
        -MMD -Ignu-efi/inc/ -Iinclude/"

AC_ARG_WITH([output-img],
            [AS_HELP_STRING([--with-output-img=<filename>], [Output .img name])],
            [output_img="$withval"],
            [output_img="kernel.elf"])


AC_ARG_WITH([kernel-elf],
            [AS_HELP_STRING([--with-kernel-elf=<filename>], [Kernel to load])],
            [kernel_elf="$withval"],
            [kernel_elf="kernel.elf"])

AC_SUBST(CFLAGS, [$CFLAGS])
AC_SUBST(OUTPUT_IMG, [$output_img])
AC_SUBST(KERNEL_ELF, [$kernel_elf])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT