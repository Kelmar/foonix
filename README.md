# Building
The build runs entirely inside Docker, so no cross-compiler toolchain needs to be built or
installed locally.

```bash
./build.sh [platform] [-o|--output <file>]
```

`platform` is `i686` or `x86_64` (defaults to `x86_64`); run `./build.sh -h` to list the
platforms available under `src/config/`. This builds a `foonix-tools` base image
(`docker/Dockerfile.tools`, the clang/cmake/grub toolchain — this layer doesn't depend on
platform, so Docker's layer cache keeps rebuilds of it fast), then builds
`docker/Dockerfile.build` on top of it for the requested platform. The resulting
`boot.<platform>.iso` and `kernel.<platform>.elf` land in `./out/`; pass `-o` to override the
ISO's filename.

Other scripts at the repo root:
- `clean.sh` — removes `./out` (`-t` also removes `build/tools`, a leftover from the legacy
  non-Docker cross-compiler flow described below).
- `test.sh` — configures/builds/runs the host-side unit tests under `src/tests` via CTest.
- `qemu.sh [platform]` — boots `out/boot.<platform>.iso` (defaults to `x86_64`) under
  `qemu-system-x86_64`, halted with a GDB stub on `:1234`.

CI (`.github/workflows/build.yml`) builds both `i686` and `x86_64` on every push/PR and
uploads the resulting ISO/ELF as workflow artifacts.

### Building without Docker

If you'd rather build directly against a host toolchain (`clang`/`clang++`, `cmake`, and
`grub-mkrescue` via `grub-pc-bin`/`grub-common`/`xorriso`), `src/` is a self-contained CMake
project and supports out-of-tree builds:

```bash
mkdir build-foonix && cd build-foonix
cmake -DCONFIG=x86_64 -G Ninja ../src
ninja
```

(`build/tool-build.sh` also exists for building a standalone `*-elf` GCC/binutils cross-compiler
into `tools/`, predating the Docker/clang build — it isn't part of the `build.sh`/CMake flow
above and is kept only for reference.)

# Some stuff that's done
* ~Global descriptor tables (although simple) are working.~
* ~Interrupts are handled.~
* ~Code for handling delays using the PIT is working.~

(Long ago broke a lot of this in favor of a rework)

# Some TODOs
## Build System
[x] Replace Makefile with CMake/Ninja
[x] Build x64
[x] Use docker so we can better control the build environment.
[ ] Better unit test system. ([doctest](https://github.com/doctest/doctest) maybe?)

## Kernel Proper
[ ] Get it so we can use a debugger with qemu.
[ ] Remove use of GRUB and switch to EFI
[ ] x64 code
[ ] Fix broken paging support
    (somewhat fixed now)
[ ] Working memory manager.
[ ] Need VFS
[ ] Make a real driver model.
[ ] Finish ATA driver.
[ ] Build a file system.
[ ] Add basic POSIX hooks for getting libc to compile/run.
[ ] Everything else that isn't done yet.

## C/C++ Library
* Use a premade libc and libc++ (at least for user space)

# Running Tests

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
