#!/bin/bash

export PREFIX=$(pwd)
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

echo "Downloading required sources ..."
if [ ! -f "binutils-2.39.tar.xz" ]; then
	wget https://ftp.gnu.org/gnu/binutils/binutils-2.39.tar.xz
fi

if [ ! -f "gcc-12.2.0.tar.xz" ]; then
	wget https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.xz
fi

mkdir -p src

echo "uncompressing binutils ..."
tar xf binutils-2.39.tar.xz -C src
echo "uncompressing gcc ..."
tar xf gcc-12.2.0.tar.xz -C src

echo "building binutils ..."
cd src
mkdir -p build-binutils
cd build-binutils
../binutils-2.39/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

echo "building gcc ..."
cd ${PREFIX}/src
mkdir -p build-gcc
cd build-gcc
../gcc-12.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc
