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

It is possible to build outside of the docker container.  To do so you will need to have
the following tools installed:
- `clang`
- `cmake`
- `grub-mkrescue`
- `grub-pc-bin`
- `xorriso` (needed by grub) 
- `ninja` (optional, you can use `make` if you want.)

Here is the basic command structure for doing a build:
```bash
mkdir -p build-foonix && cd build-foonix
cmake -DCONFIG=x86_64 -G Ninja ../foonix/src
cmake --build .
cmake --install .
```

You can select a different platform with the `-DCONFIG=<platform>` option.

This will get a basic sysroot directory structure, to get the final iso:
```bash
grub-mkrescue -o boot.iso sysroot
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
- [x] ~Global descriptor tables (although simple) are working.~
- [x] ~Interrupts are handled.~
- [x] ~Code for handling delays using the PIT is working.~

(Long ago broke a lot of this in favor of a rework)

# Some TODOs
## In Progress
- [ ] x64 code
- [ ] Fix broken paging support
    (somewhat fixed now)

## Build System
- [x] Replace Makefile with CMake/Ninja
- [x] Build x64
- [x] Use docker so we can better control the build environment.
- [ ] Better unit test system. ([doctest](https://github.com/doctest/doctest) maybe?)

## Kernel Proper
- [x] Get it so we can use a debugger with qemu.
- [ ] Remove use of GRUB and switch to EFI
- [ ] Working memory manager.
- [ ] Need VFS
- [ ] Make a real driver model.
- [ ] Finish ATA driver.
- [ ] Build a file system.
- [ ] Add basic POSIX hooks for getting libc to compile/run.
- [ ] Everything else that isn't done yet.

## C/C++ Library
- [ ] Use a premade libc and libc++ (at least for user space)

# Adding platforms
The code is structured so that new platforms can be added without a lot of fuss (hopefully).

To do so you will need a new configuration file in `src/config`.  These control some build
options and make it a bit easier to select which build is desired without having to edit
a lot of stuff in the main build system files.  See the `lint.json` file some details on
how these files are constructed.

After that you need a new folder in the `src/kernel/arch` with the name of the platform as
listed in the `config.json` file above.  This folder at a bare minimum should have a `build.cmake`
file in it that provides all the needed over rides and additions to include the files needed
for that platform.

You can look at the `src/kernel/arch/i386/build.cmake` for example.
