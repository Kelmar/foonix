#!/bin/sh

BUVER=2.18
GCCVER=3.4.6
MAKEVER=3.81
GRUBVER=

TARGET=i586-elf

cwd=`pwd`

PREFIX=$cwd/tools

# The find_XXX() functions fill these out.
MAKE=make
FETCH=

find_gmake() {
    if [ "$MAKE" != "" ] ; then
	return 0
    fi

    MAKE=`whereis gmake`
    if [ "$MAKE" != "" ] ; then
	return 0
    fi

    # This will cause BSD make to print usage, which should give us
    # an indication if we need to tell the user to install gmake
    # or not....
    mver=`make --version 2>&1 | awk '/GNU/ {print $$1}'`
    if [ "$mver" = "GNU" ] ; then
	# The system already uses GNU make as its default make.
        MAKE=`whereis make`
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

find_fetch() {
    if [ "$FETCH" != "" ] ; then
	return
    fi

    FETCH=`whereis fetch`
    if [ "$FETCH" = "" ] ; then
	FETCH=`whereis wget`

	if [ "$FETCH" = "" ] ; then
	    FETCH=`whereis ftp`
	fi
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

	tar -zxvf make-$MAKEVER.tar.gz
	mv make-$MAKEVER ..
    fi

    if [ ! -f tars/gcc-core-$GCCVER.tar.gz ] ; then
	$FETCH ftp://ftp.gnu.org/gnu/gcc/gcc-$GCCVER/gcc-core-$GCCVER.tar.gz
    fi

    if [ ! -f tars/gcc-g++-$GCCVER.tar.gz ] ; then
	$FETCH ftp://ftp.gnu.org/gnu/gcc/gcc-$GCCVER/gcc-g++-$GCCVER.tar.gz
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
	MAKE=`whereis make`
	if [ "$MAKE" = "" ] ; then
	    # We can't find ANY SORT OF MAKE!!!!
	    echo "I need some form of make!"
	    exit 1
	fi

	if [ ! -d build/make ] ; then
	    mkdir -p build/make
	fi

	cd build/make
	../../make-$MAKEVER/configure --prefix=$PREFIX
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

    if [ ! -d tools ] ; then
	mkdir tools
    fi

    if [ ! -d build/bu ] ; then
	mkdir -p build/bu
    fi

    cd build/bu

    if [ ! -f Makefile ] ; then
	../../binutils-$BUVER/configure \
		--prefix=$cwd/tools --target=$TARGET \
		--disable-nls
    fi

    $MAKE all
    $MAKE install
    cd ../..

    if [ ! -d build/gcc ] ; then
	mkdir -p build/gcc
    fi

    cd build/gcc

    if [ ! -f Makefile ] ; then
	../../gcc-$GCCVER/configure \
		--prefix=$cwd/tools --target=$TARGET \
		--disable-nls --enable-languages=c,c++ --without-headers
    fi

    $MAKE all-gcc
    $MAKE install-gcc
    cd ../..
}

kernel() {
    if ! find_gmake ; then
	if ! build_make ; then
	    echo "You will need to download GNU make."
	    echo "Try: $0 fetch"
	    exit 1
	fi	
    fi

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
