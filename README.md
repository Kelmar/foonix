# Building
You only need Docker to build the OS; the build runs entirely inside a container using Clang, 
so you do not need to wait on a cross-compiler toolchain build.

To make this a bit easier, you can use the build.sh script to kick off the build:

```bash
./build.sh [platform] [-o|--output <file>]
```

Where `platform` is currently either `i686` or `x86_64` (defaults to `x86_64`).
You can also run `./build.sh -h` to list the platforms available under `src/config/`.

You will find the resulting files in the `./out/` directory; or you can pass the `-o` flag
to override the ISO's output filename.  A `kernel.<platform>.elf` is also generated so
you can load it up with gdb and debug it.

Other scripts at the repo root:
- `test.sh` — configures/builds/runs the host-side unit tests under `src/tests` via CTest.
- `qemu.sh [platform]` — boots `out/boot.<platform>.iso` (defaults to `x86_64`) under
  `qemu-system-x86_64`

### Building without Docker

If you'd rather build directly against a host toolchain (`clang`/`clang++`, `cmake`, and
`grub-mkrescue` via `grub-pc-bin`/`grub-common`/`xorriso`), `src/` is a self-contained CMake
project and supports out-of-tree builds:

```bash
mkdir build-foonix && cd build-foonix
cmake -DCONFIG=x86_64 -G Ninja ../src
ninja
```
### Running Tests

Host-side unit tests (currently covering `libk` and `KernelArgs`) build and run natively,
independent of the Docker kernel build:

```bash
./test.sh
```

which is equivalent to:
```bash
cmake -S src/tests -B build/tests
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

# Some stuff that's done
* ~Global descriptor tables (although simple) are working.~
* ~Interrupts are handled.~
* ~Code for handling delays using the PIT is working.~

(Long ago broke a lot of this in favor of a rework)

# Some TODOs
## In Progress
[ ] x64 code
[ ] Fix broken paging support
    (somewhat fixed now)

## Build System
[x] Replace Makefile with CMake/Ninja
[x] Build x64
[x] Use docker so we can better control the build environment.
[ ] Better unit test system. ([doctest](https://github.com/doctest/doctest) maybe?)

## Kernel Proper
[x] Get it so we can use a debugger with qemu.
[ ] Remove use of GRUB and switch to EFI
[ ] Working memory manager.
[ ] Need VFS
[ ] Make a real driver model.
[ ] Finish ATA driver.
[ ] Build a file system.
[ ] Add basic POSIX hooks for getting libc to compile/run.
[ ] Everything else that isn't done yet.

## C/C++ Library
[ ] Use a premade libc and libc++ (at least for user space)

