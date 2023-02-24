AC_INIT([zebra_loader], [0.0.1], [ian@ian-moffett.com])
CROSS_ERR_MSG="Please run tools/cross.sh"

AC_CHECK_FILE([cross/bin/x86_64-elf-gcc], [], AC_MSG_ERROR([$CROSS_ERR_MSG]))
AC_CHECK_FILE([cross/bin/x86_64-elf-ld], [], AC_MSG_ERROR([$CROSS_ERR_MSG]))

CFLAGS="-fpic -ffreestanding -fno-stack-protector -fno-stack-check  \\
				-fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c   \\
        -MMD -Ignu-efi/inc/ -Iinclude/"

AC_ARG_ENABLE([graphics],
              [AS_HELP_STRING([--enable-graphics], [show graphical menu])],
              [if test "x$enableval" = "xyes"
              then
                CFLAGS="$CFLAGS -DUSE_GFX"
              fi])

AC_SUBST(CFLAGS, [$CFLAGS])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
