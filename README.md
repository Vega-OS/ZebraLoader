# ZebraLoader

ZebraLoader is a minimalistic bootloader program designed to
initialize the system and load a kernel.

## Building

For a list of options use: ``./configure --help``.
To build use: ``./configure <flags>; make``

## Wallpaper

To have a wallpaper on the bootscreen you can
build like so (for assets/wallpaper.bmp set):

``./configure --with-wallpaper-filename=wallpaper.bmp; make``

## Resolution

Don't like the resolution? Press ``+`` with SHIFT held down to get
the next resolution available!

## Custom boot entryname

For "CoolOS" you can build like so: ``./configure --with-boot-entryname=CoolOS``

## Screenshot

![screenshot](https://github.com/Vega-OS/ZebraLoader/blob/main/.github/screenshot.png?)
