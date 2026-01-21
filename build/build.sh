#!/usr/bin/bash
# Utility for downloading and compiling the build tool chain

BUVER=2.45
GCCVER=15.2.0

BUTAR=binutils-$BUVER.tar.xz
GCCTAR=gcc-$GCCVER.tar.xz

cwd=`pwd`

TOOLS="$cwd/../tools"
PREFIX="$TOOLS"
TMP="$cwd/tmp"

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
    elif exists "curl" ; then
        FETCH=curl
    fi

    if [ "$FETCH" = "" ] ; then
        echo "Unable to find \"fetch\", \"wget\" or "curl", please install one."
        exit 1
    fi
}

fetch_missing() {
    mkdir -p $TMP
    pushd $TMP

    if [ ! -f $BUTAR ] ; then
        $FETCH https://ftp.gnu.org/gnu/binutils/$BUTAR
    fi

    if [ ! -f $GCCTAR ] ; then
        $FETCH https://ftp.gnu.org/gnu/gcc/gcc-$GCCVER/$GCCTAR
    fi

    popd
}

extract_tarballs() {
    pushd $TMP

    if [ ! -d binutils-$BUVER ] ; then
        echo -n "Extracting binutils..."
        tar -Jxf $BUTAR
        echo " [DONE]"
    fi

    if [ ! -d gcc-$GCCVER ] ; then
        echo -n "Extracting gcc..."
        tar -Jxf $GCCTAR
        echo " [DONE]"
    fi

    echo "Checking GCC prereqs."
    cd gcc-$GCCVER
    . ./contrib/download_prerequisites

    popd
}

configure_binutils() {
    ../binutils-$BUVER/configure "--prefix=$PREFIX" "--target=$TARGET" \
        --with-sysroot --disable-nsl --disable-werror
}

build_binutils() {
    pushd $TMP

    mkdir -p build-bu
    cd build-bu

    if [ ! -f Makefile ] ; then
        configure_binutils
    fi

    $MAKE ${JOBS} all
    $MAKE install

    popd
}

configure_gcc() {
    if [ "${TARGET}" = "x86_64-elf" ] ; then
        if [ ! -f "../gcc-$GCCVER/gcc/config/i386/t-x86-64-elf" ] ; then
            echo "Patching no red zone into GCC"

            mkdir -p ../gcc-$GCCVER/gcc/config/i386
            cat > ../gcc-$GCCVER/gcc/config/i386/t-x86-64-elf << __NR_EOF
MULTILIB_OPTIONS += mno-red-zone
MULTILIB_DIRNAMES += no-red-zone
__NR_EOF

            sed -i.bak '/^x86_64-\*-elf\*)/a \\ttmake_file="${tmake_file} i386/t-x86_64-elf"' ../gcc-$GCCVER/gcc/config.gcc
        fi
    fi

    ../gcc-$GCCVER/configure \
        "--prefix=$PREFIX" "--target=$TARGET" \
        --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx
}

build_gcc() {
    pushd $TMP

    mkdir -p build-gcc
    cd build-gcc

    if [ ! -f Makefile ] ; then
        configure_gcc
    fi

    $MAKE ${JOBS} all-gcc
    $MAKE ${JOBS} all-target-libgcc
    $MAKE ${JOBS} all-target-libstdc++-v3

    $MAKE install-gcc
    $MAKE install-target-libgcc
    $MAKE install-target-libstdc++-v3

    popd
}

build_tools() {
    if [ ! -f $PREFIX/bin/${TARGET}-ld ] ; then
        build_binutils
    else
        echo "binutils up to date"
    fi

    if [ ! -f $PREFIX/bin/${TARGET}-gcc ] ; then
        build_gcc
    else
        echo "gcc up to date"
    fi
}

TARGET=$1-elf
JOBS="-j4"

# Look for fetch or wget first
find_fetch
fetch_missing
extract_tarballs
build_tools
