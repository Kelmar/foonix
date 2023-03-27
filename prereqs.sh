#!/bin/sh
# Utility for downloading and compiling the build tool chain

BUVER=2.40
GCCVER=12.2.0
GRUBVER=

BUTAR=binutils-$BUVER.tar.xz
GCCTAR=gcc-$GCCVER.tar.xz

TARGET=i686-elf

cwd=`pwd`

TOOLS="$cwd/tools"
PREFIX="$TOOLS/local"
TARS="$TOOLS/tars"

# The find_XXX() functions fill these out.
FETCH=

# For now we just hard code to "make" may need to auto discover for "gmake" on BSD systems.
MAKE=make

exists() {
    type "$1" > /dev/null 2> /dev/null
}

find_fetch() {
    if exists "fetch" ; then
        FETCH=fetch
    elif exists "wget" ; then
        FETCH=wget
    fi

    if [ "$FETCH" = "" ] ; then
        echo "Unable to find \"fetch\" or \"wget\" please install one."
        exit 1
    fi
}

fetch_missing()
{
    mkdir -p $TARS
    cd $TARS

    if [ ! -f $BUTAR ] ; then
        $FETCH https://ftp.gnu.org/gnu/binutils/$BUTAR
    fi

    if [ ! -f $GCCTAR ] ; then
        $FETCH https://ftp.gnu.org/gnu/gcc/gcc-$GCCVER/$GCCTAR
    fi

    cd $CWD
}

build_binutils() {
    cd $TARS

    if [ ! -d binutils-$BUVER ] ; then
        tar -Jxf $BUTAR
    fi

    if [ ! -d gcc-$GCCVER ] ; then
        tar -Jxf $GCCTAR
    fi

    mkdir -p build-bu
    cd build-bu

    if [ ! -f Makefile ] ; then
        ../binutils-$BUVER/configure --prefix=$PREFIX --target=$TARGET \
           --with-sysroot --disable-nsl --disable-werror
    fi

    $MAKE all
    $MAKE install

    cd $CWD
}

build_gcc() {
    cd $TARS

    mkdir -p build-gcc
    cd build-gcc

    if [ ! -f Makefile ] ; then
        ../gcc-$GCCVER/configure \
            --prefix=$PREFIX --target=$TARGET \
            --disable-nls --enable-languages=c,c++ --without-headers
    fi

    $MAKE all-gcc
    $MAKE all-target-libgcc
    $MAKE install-gcc
    $MAKE install-target-libgcc

    cd $CWD
}

build_tools() {
    if [ ! -f $PREFIX/bin/i686-elf-ld ] ; then
        build_binutils
    else
        echo "binutils up to date"
    fi

    if [ ! -f $PREFIX/bin/i686-elf-gcc ] ; then
        build_gcc
    else
        echo "gcc up to date"
    fi
}

# Look for fetch or wget first
find_fetch
fetch_missing
build_tools
