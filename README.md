Information goes here I think....

Lots of stuff is still broken with everything including the build.

Some general help though:

These tools still must be built by hand; good luck.

= Some stuff that's done =
* Global descriptor tables (although simple) are working.
* Interrupts are handled.
* Code for handling delays using the PIT is working.

# Some TODOs
* Makefiles need some serious work.
* Architecture specific code needs to be seperated out.
* Some functions need to be moved into a common library.
* Finish ATA driver.
* Build a file system.
* Add basic POSIX hooks for getting newlib/glibc to compile/run.
* Make a real driver model.
* Fix broken paging support, get a working memory manager.
* Everything else that isn't done yet.
* C++ support?  Maybe?

# Building
Everything is controlled through the build.sh script for now.

You can fetch the tool chain with "build.sh fetch"

And then build the tools with "build.sh tools"

Finally the kernel itself can be built with "build.sh kernel"
