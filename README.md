Lots of stuff is still broken with everything including the build.

Some general help though:

These tools still must be built by hand; good luck.

The build script will download and build the cross compiler tools for you:
./build.sh tools

# Some stuff that's done
* Global descriptor tables (although simple) are working.
* Interrupts are handled.
* ~Code for handling delays using the PIT is working.~

# Some TODOs
## Build System
* Currently replacing Makefile with CMake/Ninja
* Some functions need to be moved into a common library.
* Building is a bit less ugly:

You will need CMake and a compiler that can build GCC 12.2.0 and BinUtils 2.40

The cross compiler can be downloaded and built with the following command:
```bash
./scripts/prereqs.sh
```

Then you can build the kernel with:
```bash
cmake .
make
```

Or if you have Ninja installed:
```bash
cmake -G Ninja .
ninja
```

## Kernel Proper
* Fix broken paging support, get a working memory manager.
* Need VFS
* Make a real driver model.
* Finish ATA driver.
* Build a file system.
* Add basic POSIX hooks for getting libc to compile/run.
* Everything else that isn't done yet.

## C/C++ Library
* Better C++ library
* Use a premade libc and libc++ (at least for user space)

# Building
Everything is controlled through the build.sh script for now.

You can fetch the tool chain with "build.sh fetch"

And then build the tools with "build.sh tools"

Finally the kernel itself can be built with "build.sh kernel"
