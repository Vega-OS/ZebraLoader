#!/usr/bin/env bash
################################################################
# A script to build a GNU GCC and Binutils cross-compiler
# Based on https://wiki.osdev.org/GCC_Cross-Compiler
#
# Copyright (c) 2022, Quinn Stephens.
################################################################

################################################################
# Configuration
################################################################

# Exit if there is an error
set -e

# Target to use
TARGET=x86_64-elf

# Versions to build
# Always use the latest working version (test before updating)
BUT_VER=2.40
GCC_VER=12.2.0

# Tar file extension to use
# Always use the one with the smallest file size (check when updating version)
BUT_EXT=xz
GCC_EXT=xz

# Multicore builds
# Currrently automatic using nproc
CORES=$(nproc)
LOAD=$(($(nproc) - 1))

PREFIX="$(pwd)/cross"
export PATH="$PREFIX/bin:$PATH"

clear
echo "Building $TARGET Binutils $BUT_VER and GCC $GCC_VER..."
echo "Cores: $CORES, load: $LOAD"

################################################################
# Source tarballs
################################################################

BUT_TARBALL=binutils-$BUT_VER.tar.$BUT_EXT
GCC_TARBALL=gcc-$GCC_VER.tar.$GCC_EXT

mkdir -p build_cross
cd build_cross

# Download tarballs
echo "Downloading Binutils tarball..."
if [ ! -f $BUT_TARBALL ]; then
    wget https://ftp.gnu.org/gnu/binutils/$BUT_TARBALL
fi

echo "Downloading GCC tarball..."
if [ ! -f $GCC_TARBALL ]; then
    wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/$GCC_TARBALL
fi

# Unzip tarballs
echo "Unzipping Binutils tarball..."
tar -xf $BUT_TARBALL
echo "Unzipping GCC tarball..."
tar -xf $GCC_TARBALL

################################################################
# Building
################################################################

echo "Removing old build directories..."
rm -rf build_gcc build_binutils

# Build binutils
mkdir build_binutils
cd build_binutils
clear
echo "Configuring Binutils..."
../binutils-$BUT_VER/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
clear
echo "Building Binutils..."
make -j$CORES -l$LOAD
clear
echo "Installing Binutils..."
make install -j$CORES -l$LOAD
cd ..

# Build gcc
cd gcc-$GCC_VER
clear
echo "Downloading prerequisites for GCC..."
contrib/download_prerequisites
cd ..
mkdir build_gcc
cd build_gcc
clear
echo "Configuring GCC..."
../gcc-$GCC_VER/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
clear
echo "Building all-gcc..."
make all-gcc -j$CORES -l$LOAD
clear
echo "Building all-target-libgcc..."
make all-target-libgcc -j$CORES -l$LOAD
clear
echo "Installing GCC..."
make install-gcc -j$CORES -l$LOAD
clear
echo "Installing target-libgcc..."
make install-target-libgcc -j$CORES -l$LOAD
cd ..

################################################################
# Cleanup & basic testing
################################################################

cd ..
clear
echo "Removing build directory..."
rm -rf build_cross

echo "Build complete, binaries are in $PREFIX/bin"

echo "Testing GCC..."
$TARGET-gcc -v

echo "Testing LD..."
$TARGET-ld -v

echo "Done!"
