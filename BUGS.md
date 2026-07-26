# Known bugs

Found by a manual code-review pass over the currently-active code (VM/boot memory, the debug
console path, the freestanding libc, and the i386 arch layer). Ordered roughly by severity/blast
radius. Check items off as they're fixed.

## Critical — memory corruption in code paths that run today

- [x] **`sys/libk/stdio/vsnprintf.c`** — the workhorse behind every `Debug::PrintF`/`kpanic`/`kassert` call.
  - Several switch cases wrote `sbuf[s++] = ...` with no bounds check at all: the `'%'`/default
    case, `'c'`, and the sign character.
  - Once `s` had overrun past `slen` via one of those, `width > (slen - s - 1)` and
    `_s_strncat(sbuf + s, slen - s, ...)` compute on `size_t` — `slen - s` underflows to a huge
    number, so the clamp that's supposed to stop overflow instead permits massive writes via
    `memset`/`_s_strncat`.
  - `exit: sbuf[s] = '\0';` wrote `sbuf[slen]` (one past the caller's buffer) whenever the
    literal-copy loop exactly filled it.
  - `Debug::vPrintF` (`sys/kernel/src/debug.cpp:22`) formats into a 512-byte **stack** buffer, so
    this is a stack-smashing bug reachable from ordinary debug output, not just an edge case.
  - Fix: check `s < slen` before every write, and clamp all `slen - s` arithmetic against
    `s >= slen` before subtracting.

- [x] **`sys/libk/string/memmove.c`** — the backward-copy branch started at `i = size` instead of
  `size - 1`, so it read/wrote `dst[size]`/`src[size]` (one past both buffers) and never copied
  index 0. `memmove` is the primitive that's supposed to be *safe* for overlapping ranges; as
  written it corrupted memory whenever `dst > src`.

- [x] **`sys/include/kernel/bitmap.h`** — `Set`/`Clear`/`operator[]` bounds-checked with
  `if (index > BitCount) return;`. Valid indices are `0..BitCount-1`, so `index == BitCount` slipped
  through and wrote `m_items[ArrayCount]`, one word past the array. `Bitmap` backs
  `SlabAllocator`'s per-page allocation tracking.

- [x] **`sys/libk/string/_s_strncpy.c`** — `dst[mlen] = '\0'` could write `dst[dsz]`, one byte
  past the destination when the copy exactly filled the buffer; `mlen` was never capped to
  `dsz - 1`.

## High — silently wrong behavior

- [x] **`sys/libk/string/strncpy.c`** — didn't behave like the C standard function it's named
  after: searched for an existing NUL in `dst` and *appended* `src` after it (that's `strncat`'s
  job), and returned `NULL` if `dst` wasn't already terminated. Rewritten to copy from the start
  and NUL-pad the remainder. That rewrite initially had its own off-by-one (`++ssz` ran before
  being used for both the `memset` destination and the remaining count, leaving `dst[L+1]`
  — the byte right after the copied string's own terminator — permanently untouched/uninitialized,
  or skipping the padding entirely when only one byte of it was needed); fixed by dropping the
  premature increment. Verified via `tests/test_bounded_string.c`'s
  `test_strncpy_pads_remainder_with_nul`, which checks the full padded region byte-for-byte.

- [x] **`sys/libk/string/strncat.c`** — `memcpy`'d straight over the **start** of `buf`
  (overwriting existing content instead of appending after it) and never wrote a terminator.
  Fixed: computes `blen` (existing content length via `strnlen`), bails out if `buf` isn't
  NUL-terminated within `size`, appends at `buf + blen`, and always leaves the result terminated
  within bounds.

- [x] **`sys/kernel/arch/i386/i386/arch_vm.cpp`** — `ReleaseRealMemory`'s
  `if (index == 0 || offset == 0) return;` refused to ever release any page in the first 32-page
  group, and every 32nd page anywhere; `AllocRealMemory`'s inner scan only checked `offset < 8` of
  a 32-bit word (24 of every 32 tracked pages were unreachable), and its outer loop indexed the
  map array before checking the bound, reading one word past the end once memory was exhausted.
  Rewritten to use `Bitmap<>` (the same template fixed earlier) instead of hand-rolled dword/bit
  math. Along the way, also fixed: `AllocRealMemory`/`ReleaseRealMemory` now consistently deal in
  real page-aligned addresses (`index * PAGE_SIZE`) rather than a raw page index, which actually
  matches `arch_vm.h`'s documented contract ("Returns a page aligned real memory address") --
  the pre-`Bitmap` code never did, despite the doc comment. Verified with a `-fsyntax-only`
  cross-compile (`-Wall -Wextra -Wpedantic`, clean) since this isn't exercised by the native libk
  test suite (nothing calls these functions yet).

- [x] **`sys/kernel/src/vm/vm.cpp`** — `BootPageAllocator::NewBlock` checked
  `MemoryMap[index].Length >= index` (loop counter) instead of `>= byteSize` (the actual requested
  size). Combined with the unconditional `Length -= byteSize` right after, an over-large request
  underflowed `Length` and corrupted that memory-map entry. Fixed: compares against `byteSize`
  directly, so `Length -= byteSize` can only run once enough room is already confirmed. Verified
  with a clean `-fsyntax-only` cross-compile.

- [x] **`sys/kernel/arch/x86/dev/tty.cpp`**
  - `set_loc()`: `row * VGA_WIDTH * col` should be `row * VGA_WIDTH + col` — fixed. Same function
    also wrote `CUR_LOC_LO` to the data port instead of the index port, so the low-byte cursor
    write never reached its own register (both writes landed on whatever register
    `CUR_LOC_HI` had last selected) — also fixed, now does select-register/write-data for both
    the high and low byte correctly. (Not fixed and out of scope here: `terminal_io` is a global
    initialized to `0` and never set to a real CRT controller port anywhere, since `terminal_init()`
    — the function that would set it up — is `#if 0`'d out. So `set_loc`'s writes don't reach real
    VGA hardware yet regardless; that's pre-existing "not wired up," not a bug introduced or fixed
    here.)
  - `terminal_dump()`: `idx` was never incremented in the loop, so every hex digit overwrote the
    same screen cell. Fixed (`idx++`). Verified both with a clean `-fsyntax-only` cross-compile.

## Medium

- [x] **`sys/libk/string/memcmp.c`** — found while writing unit tests for it. The `else if
  (b[i] > a[i])` branch was logically identical to `a[i] < b[i]` (same comparison, operands
  swapped), so it was unreachable dead code — the function could only ever return `-1` or `0`; it
  never detected `a > b`. Fixed: returns `(int)a[i] - (int)b[i]` at the first differing byte
  (safe -- both are widened from `unsigned char`, so the subtraction can't overflow `int`).

- [ ] **`sys/kernel/src/kernel_args.cpp`** — `AddFreeMemory`'s three branches miss the case where a
  record starts *before* the kernel and ends *inside* it
  (`addr < Base && Base < end <= End()`); it falls through unadjusted, so memory that overlaps the
  kernel image gets reported as fully free.

- [ ] **`sys/include/kernel/allocator.h`** — `SlabAllocator<T>` never overrides
  `Allocate()`/`Release()`, so it's still abstract and can't be instantiated anywhere it's
  actually needed.

- [x] **`sys/libk/stdio/vsnprintf.c`** — the `exit:` clamp (`s = s < (slen - 1) ? s : slen - 1;`)
  didn't account for `slen == 0`: `slen - 1` underflowed a `size_t`, so `s` stayed `0` and
  `sbuf[0] = '\0'` still ran even though the caller claimed zero usable bytes. Fixed with an early
  `if (slen == 0) return 0;` at the top -- as a side effect, every path that now reaches `exit:` is
  guaranteed `slen >= 1`, so the clamp itself can never underflow either. Verified via
  `tests/test_vsnprintf.c`'s `test_zero_size_buffer`.

- [ ] **`sys/include/utility`** — the shim `std::forward` has only one overload (taking `T&`), so
  it always casts to an rvalue reference regardless of the deduced/explicit `T` — it behaves like
  `std::move`, not true forwarding. No current call site is bitten by this, but the first generic
  forwarding wrapper written against it will silently force-move lvalues.

- [x] **`sys/include/kernel/flow.h` vs `sys/include/sys/assert.h`** — found while syntax-checking
  `vm.cpp`. Both headers independently declared `kpanic`/`kassert` and `#define ASSERT(...)` with
  the same expansion -- full duplication, unnoticed until the `__FUNCTION__` -> `__func__` fix
  (see the `vsnprintf.c` entry above) only touched `sys/assert.h`'s copy, breaking GCC's silent
  tolerance for byte-identical macro redefinition.
  - First attempt made it worse rather than better: `sys/assert.h` was restructured to branch on
    `__is_foo_kernel`/`__is_libk` (routing to `kpanic`/`kassert` for real kernel/libk builds, or a
    host `fprintf`/`abort()` fallback otherwise -- a genuinely good idea, see below) and to
    unconditionally `#include "kernel/flow.h"` in the kernel branch, but `flow.h` itself was left
    untouched, still defining its own competing `ASSERT`. That guaranteed the clash in *every*
    kernel/libk translation unit that includes `sys/assert.h`, not just ones that happened to
    include both headers independently. Confirmed with `-Werror`:
    `sys/include/sys/assert.h:19:10: error: 'ASSERT' redefined [-Werror]`.
  - Fixed by dropping the duplicate `#define ASSERT(...)` from `flow.h` (kept its `khalt`/`kpanic`/
    `kassert` declarations, since `idt.cpp` and `cppfill.cpp` use those directly) -- `sys/assert.h`
    is now the sole owner of the macro. Verified with `-Werror` cross-compiles of `vsnprintf.c`
    (`-D__is_libk`), `vm.cpp`, `bus.cpp`, and `vm_page.cpp` (`-D__is_foo_kernel`), all clean.
  - The host-branch fallback (`fprintf`/`abort()` for non-kernel builds) doesn't `#include
    <stdio.h>`/`<stdlib.h>` itself, so `fprintf`/`stderr`/`abort` are undeclared wherever it's
    taken outside a kernel/libk build -- caught this via the native libk test suite, which
    doesn't define `__is_libk`. Rather than make the host branch self-sufficient (which runs into
    the same `sys/cdefs.h`-shadowing trap noted in `tests/kstubs.c` if `sys/include` is also on
    the compiler's path), `tests/CMakeLists.txt` now defines `__is_libk` for `libk_under_test` --
    which is worth doing for fidelity with the real libk build regardless, and lets `ASSERT` route
    through the already-working `kpanic`/`kassert` stubs in `kstubs.c` instead.
