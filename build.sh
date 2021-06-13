#!/bin/sh

BUVER=2.36
GCCVER=11.1.0
MAKEVER=4.3
GRUBVER=

TARGET=i686-elf

cwd=`pwd`

PREFIX=$cwd/tools

# The find_XXX() functions fill these out.
MAKE=make
FETCH=

find_gmake() {
    if exists "gmake" ; then
        MAKE=gmake
        return 0
    fi

    # This will cause BSD make to print usage, which should give us
    # an indication if we need to tell the user to install gmake
    # or not....
    mver=`make --version 2>&1 | awk '/^GNU/ {print $1}'`
    if [ "$mver" = "GNU" ] ; then
	# The system already uses GNU make as its default make.
        MAKE=make
	return 0
    fi

    # Check our tool's directory
    if [ -f $PREFIX/bin/make ] ; then
	    MAKE=$PREFIX/bin/make
	    return 0
    fi

    # GNU make was not found.
    return 1
}

exists() {
    type "$1" > /dev/null 2> /dev/null
}

find_fetch() {
    if exists "fetch" ; then
        FETCH=fetch
    elif exists "wget" ; then
        FETCH=wget
    elif exists "ftp" ; then
        FETCH=ftp
    fi

    if [ "$FETCH" = "" ] ; then
        echo "Unable to find \"fetch\", \"wget\", or \"ftp\" please install one."
        exit 1
    fi
}

fetch_missing()
{
    find_fetch

    if [ ! -d tars ] ; then
	    mkdir tars
    fi

    cd tars

    if ! find_gmake ; then
        if [ ! -f make-$MAKEVER.tar.gz ] ; then
            # Get a copy from GNU's FTP site.
            $FETCH ftp://ftp.gnu.org/gnu/make/make-$MAKEVER.tar.gz
        fi
    fi

    if [ ! -f binutils-$BUVER.tar.gz ] ; then
        $FETCH ftp://ftp.gnu.org/gnu/binutils/binutils-$BUVER.tar.gz
    fi

    if [ ! -f gcc-$GCCVER.tar.gz ] ; then
        $FETCH ftp://ftp.gnu.org/gnu/gcc/gcc-$GCCVER/gcc-$GCCVER.tar.gz
    fi

    cd ..
}

check() {
    find_fetch
    echo "FETCH: $FETCH"

    if ! find_gmake ; then
	    echo "You will need to download GNU make."
    else
	    echo "MAKE: $MAKE"
    fi

    echo "Build environment seems OK"
}

exit_cleanup() {
    export PATH=$OLDPATH
}

build_make() {
    if [ -d make-$MAKEVER ] ; then
	    # Make can be built by us!
        if [ ! -d tars/make-$MAKEVER ] ; then
            cd tars
            tar -zxf make-$MAKEVER.tar.gz
            cd ..
        fi

        if [ ! -d build/make ] ; then
            mkdir -p build/make
        fi

        cd build/make

            if [ ! -f Makefile ] ; then
                ../../tars/make-$MAKEVER/configure --prefix=$PREFIX
            fi

            $MAKE
        $MAKE install

        cd ../..

        MAKE="$PREFIX/bin/make"

        return 0
    else
	    return 1
    fi
}

tools() {
    if ! find_gmake ; then
        if ! build_make ; then
            echo "You will need to download GNU make."
            echo "Try: $0 fetch"
            exit 1
        fi
    fi

    if [ ! -d tars/binutils-$BUVER ] ; then
        cd tars
        tar -zxf binutils-$BUVER.tar.gz
        cd ..
    fi

    if [ ! -d tars/gcc-$GCCVER ] ; then
        cd tars
        tar -zxf gcc-$GCCVER.tar.gz
        cd ..
    fi

    if [ ! -d tools ] ; then
	    mkdir tools
    fi

    if [ ! -d build/bu ] ; then
	    mkdir -p build/bu
    fi

    cd build/bu

    if [ ! -f Makefile ] ; then
        ../../tars/binutils-$BUVER/configure \
            --prefix=$PREFIX --target=$TARGET \
            --with-sysroot --disable-nls --disable-werror
    fi

    $MAKE all
    $MAKE install
    cd ../..

    if [ ! -d build/gcc ] ; then
	    mkdir -p build/gcc
    fi

    cd build/gcc

    if [ ! -f Makefile ] ; then
	    ../../tars/gcc-$GCCVER/configure \
		    --prefix=$PREFIX --target=$TARGET \
		    --disable-nls --enable-languages=c,c++ --without-headers
    fi

    $MAKE all-gcc
    $MAKE all-target-libgcc
    $MAKE install-gcc
    $MAKE install-target-libgcc
    cd ../..
}

kernel() {
    set -e
    . ./headers.sh

    $MAKE
}

help() {
    echo "USAGE: $0 [check|clean|tools|fetch|kernel]"
    echo ""
    echo "  check : Check the build environment."
    echo "  clean : Clean up the build tree."
    echo "  tools : Build cross compiling tools."
    echo "  fetch : Fetch missing tar files."
    echo "  kernel: Build the kernel."
}

# Save PATH variable so we can restore it when we're done.
OLDPATH=$PATH
export PATH=$PATH:$PREFIX/bin

trap "{ exit_cleanup; exit 0; }" EXIT

case "$1" in
check)
	check
	;;
fetch)
	fetch_missing
	;;
tools)
	tools
	;;
kernel)
	kernel
	;;
*)
	help
	;;
esac

exit_cleanup
