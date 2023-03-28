# Building
Build is a bit less ugly now, but still not great.

You will need CMake and a tool chain that can build GCC and BinUtils.

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

# Some stuff that's done
* Global descriptor tables (although simple) are working.
* Interrupts are handled.
* ~Code for handling delays using the PIT is working.~

# Some TODOs
## Build System
* Currently replacing Makefile with CMake/Ninja

## Kernel Proper
* Fix broken paging support, get a working memory manager.
* Need VFS
* Make a real driver model.
* Finish ATA driver.
* Build a file system.
* Add basic POSIX hooks for getting libc to compile/run.
* Everything else that isn't done yet.

## C/C++ Library
* Use a premade libc and libc++ (at least for user space)
