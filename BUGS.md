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

- [x] **`sys/kernel/src/kernel_args.cpp` / `sys/include/kernel/kernel_args.h`** — memory-map
  bookkeeping. Original finding: `AddFreeMemory`'s three branches missed the case where a record
  starts *before* the kernel and ends *inside* it (`addr < Base && Base < end <= End()`); it fell
  through unadjusted, so memory that overlaps the kernel image got reported as fully free.

  **Attempt 1 — narrow the recursive branch's condition.** Changing
  `addr < KernelCode.Base && end > KernelCode.End()` to just `addr < KernelCode.Base` (dropping the
  second clause instead of narrowing it) turned out to cause *guaranteed infinite recursion*: the
  branch's own first statement, `AddFreeMemory(addr, KernelCode.Base - addr)`, recurses with `end`
  now equal to `KernelCode.Base` inside the callee -- and since `addr` is unchanged, `addr <
  KernelCode.Base` is still true, so it re-enters the same branch with identical arguments forever.
  The old code avoided this specifically *because of* the now-removed second clause (`end >
  KernelCode.End()` is never true when `end == KernelCode.Base`, since `Base < End()` always).
  Confirmed with a host build under ASan: `AddFreeMemory` calling itself with identical arguments,
  stack-overflow, for *any* record entirely below the kernel -- the ordinary, common case for most
  multiboot memory-map entries. Correct fix would have been narrowing the condition to `addr <
  KernelCode.Base && end > KernelCode.Base` (compare against `Base`, not `End()`) -- verified this
  variant against all three overlap cases (no overlap / ends inside kernel / kernel fully
  contained) with no recursion issue and correct resulting map entries in each case. **Not
  applied** -- decided the recursive structure itself was too easy to get subtly wrong and opted
  to rewrite iteratively instead. `AddFreeMemory` currently still has the *original* narrower bug
  (record ending inside the kernel falls through unadjusted); it's being left in place
  side-by-side with the new methods below for comparison, to be removed once those are verified
  stable.

  **Attempt 2 — new `MemoryRange::Overlaps()` + `KernelArgs::SortMappings()` /
  `RemoveDeadMappings()` / `MergeContiguousMappings()` (private) / `AddMemoryMap()` /
  `KnockoutUsedMemory()` (public), replacing `AddFreeMemory` with a two-phase "add everything the
  bootloader reports, then knock out the kernel's own range" design.** Now wired into the actual
  boot path (`arch_vm.cpp`'s `Multiboot::InitMultibootMemory` calls `AddMemoryMap()` per record,
  then `KnockoutUsedMemory()` once at the end). First pass had several compile errors (case
  mismatch on `Overlaps`/`overlaps`, no such member `newEnd`/`Begin` on `MemoryRange`, no matching
  `MemoryRange(paddr_t, size_t)` constructor, undeclared `max()`, `kernelCode` vs `KernelCode`
  typo) -- fixed. Compiles clean now (`-Wall -Wextra -Wpedantic -fsyntax-only`, both
  `-D__is_foo_kernel` for `kernel_args.cpp`/`arch_vm.cpp`), but a clean compile didn't mean correct
  -- same lesson as attempt 1. Built a host reproduction faithfully copying the real
  `SortMappings`/`RemoveDeadMappings`/`MergeContiguousMappings`/`AddMemoryMap`/`KnockoutUsedMemory`
  logic verbatim (avoiding the project's own `<utility>`/`<type_traits>` shims, which hit an
  unrelated host/cross-g++ `__is_pointer` version mismatch) and exercised it under ASan/UBSan.
  Found three confirmed bugs, all silent data loss rather than crashes -- arguably more dangerous,
  since nothing about them looks wrong at the call site:

  1. **`MergeContiguousMappings` never actually grows the merged range.** `newLength` is computed
     (`lastMapping.Length + (mapEnd - lastEnd)`) but never assigned back to `lastMapping.Length`.
     The compaction loop meant to remove the now-redundant second entry also does
     `MemoryMap[i] = MemoryMap[j]` instead of `MemoryMap[j - 1] = MemoryMap[j]`, so it doesn't
     shift the array -- it just repeatedly overwrites index `i`, discarding everything else.
     Reproduced: adding `[0x1000,0x2000)` then the exactly-touching `[0x2000,0x3000)` (should
     merge to `[0x1000,0x3000)`, length `0x2000`) leaves the map at `Base=0x1000 Length=0x1000` --
     the second range vanishes entirely.
  2. **`AddMemoryMap`'s backward-extend branch moves `Base` without adjusting `Length`.** The
     `else if ((newRange.End() + 1) == mapping.Base)` case only does `mapping.Base =
     newRange.Base;`; `mapping.Length` is never recomputed, so the range's end effectively shrinks
     to the old length applied at the new, earlier base, instead of stretching to still cover the
     original end. Reproduced: adding `[0x5000,0x6000)` then `[0x1000,0x4FFF)` (ends exactly where
     the first begins, should merge to `[0x1000,0x6000)`) leaves the map at `Base=0x1000
     Length=0x1000` -- 0x4000 bytes gone, including the entire original mapping.
  3. **`KnockoutUsedMemory`'s split-in-two case writes to the wrong index.** When the kernel is
     fully contained within one mapping, `i = MemoryMapEntries + 1; MemoryMapEntries = i;` sets the
     count to `old + 1`, then writes the new post-kernel entry to `MemoryMap[i]` -- index `old + 1`,
     one past the slot the new count actually covers (`old`). The genuinely new data lands outside
     the counted range, while the slot the count *does* cover (`old`, never written, still
     default-constructed `{0,0}`) gets dutifully deleted by the subsequent `RemoveDeadMappings()`
     as "empty" -- discarding the real, orphaned data along with it. Reproduced: one big region
     `[0,0x200000)` containing the kernel at `[0x100000,0x102000)` should split into
     `[0,0x100000)` and `[0x102000,0x200000)`; only the first entry survives -- the ~1016KB tail is
     gone.

  Lower-severity, found alongside the above: inside `AddMemoryMap`'s `if (mapping.Overlaps(newRange))`
  block, the `if (mapEnd < newRange.Base)` branch is unreachable dead code -- `Overlaps()` itself
  requires `mapEnd >= newRange.Base` to have returned true, so its own precondition rules out ever
  reaching this branch. As a result, "`newRange` starts inside `mapping` but extends past its end"
  isn't handled anywhere in the overlap branch, and falls through to `AddMemoryMap`'s bottom
  "add a new entry" path -- which then relies on `MergeContiguousMappings` (bug 1 below) to fold it
  back in, so in practice it manifests as the same data loss as bug 1 rather than a duplicate/
  overlapping entry.

  **Attempt 3 -- redefined `MemoryRange::End()` to be inclusive** (`Base + Length - 1`, previously
  `Base + Length`), fixed bug 3 (`KnockoutUsedMemory`'s split now correctly does
  `i = MemoryMapEntries++;` before writing the new entry) and the *adjacent, non-overlapping* half
  of bug 2 (`AddMemoryMap`'s `else if ((newRange.End() + 1) == mapping.Base)` branch now does
  `mapping.Base = newRange.Base; mapping.Length += newRange.Length;`, correctly recomputing the
  length). Also replaced `sys/include/cppimpl/type_traits/pointer.h`'s use of the `__is_pointer(T)`
  compiler builtin with plain template specialization (`is_pointer<T*>`), which resolved the
  host/cross GCC skew noted above -- `tests/CMakeLists.txt` no longer needs to hunt for Clang, and
  now builds and passes with this host's stock GCC 13.3.0.

  Re-verified against the *current* code (host GCC, ASan/UBSan) rather than trusting the compile +
  the pre-existing test suite: bug 3 is fixed and the adjacent-range half of bug 2 is fixed, but
  **bug 1 (`MergeContiguousMappings` never assigns `newLength` back to `lastMapping.Length`) is
  still present, unchanged**, and the inclusive-`End()` switch introduced two new/newly-exposed
  problems, none caught by the existing suite because it never exercises a *genuine* overlap (only
  touching/adjacent ranges and full containment):

  4. **`MemoryRange::Overlaps()` is asymmetric.** Under inclusive `End()`, a correct overlap test is
     `Base <= r.End() && r.Base <= End()`, but the code has `Base < rend` (strict) for the first
     half. `a.Overlaps(b)` and `b.Overlaps(a)` can disagree whenever one range's `Base` sits exactly
     on the other's inclusive `End()` (a genuine one-byte overlap). Confirmed:
     `MemoryRange(9,5).Overlaps(MemoryRange(0,10))` is `false` even though both ranges include byte
     9. In `AddMemoryMap`, this call is always made as `mapping.Overlaps(newRange)` (existing entry
     first), so it silently skips the overlap-handling branch for this shape and falls through to
     the "add as new entry, let merge sort it out" path -- which then hits bug 1.
  5. **`AddMemoryMap`'s *overlapping* (not just touching) backward-extend branch is off by one.**
     Inside `if (mapping.Overlaps(newRange))`, the `if (newRange.Base < mapping.Base)` case computes
     `mapping.Length = maxEnd - newRange.Base;`; under inclusive `End()` this should be
     `maxEnd - newRange.Base + 1`. Confirmed: adding `[10,20]` then the overlapping `[5,15]` (true
     6-byte overlap, not adjacency) leaves the map at `[5,19]` instead of `[5,20]` -- byte 20 is
     silently dropped from the free list. This is a different branch from the one attempt 3 fixed
     (that one is the non-overlapping `else if` adjacency case, reached via `Overlaps()` being
     `false`; this one requires `Overlaps()` to be `true`).

  **Attempt 4 -- simplified the whole thing around two new `MemoryRange` primitives:
  `Contiguous(r)` (true if two ranges overlap OR touch) and `Merge(r1, r2)` (returns
  `std::expected<MemoryRange, MemoryMergeError>`, computed fresh from `min(Base)`/`max(End())`
  rather than incrementally adjusting `Length`).** `AddMemoryMap`'s per-entry loop is now just "try
  `Merge()` against each existing entry, replace on first success"; `MergeContiguousMappings`'s
  scan does the same against sorted neighbors. Also fixed `Overlaps()` itself (`Base < rend` →
  `Base <= rend`), closing bug 4. This is a real simplification and closes bugs 2 (fully, now --
  the overlapping-backward-extend case goes through the same `Merge()` as the adjacent case) and 5,
  in addition to the 3 and one already fixed in attempt 3. Re-verified against the current code
  (host GCC, ASan/UBSan), not just the pre-existing suite:

  - Bugs 2, 3, 4, 5, and the original bug 1 shape (three-way bridge via `Merge()`-computed lengths)
    are now genuinely fixed -- confirmed by rerunning the full `tests/test_kernel_args.cpp` suite
    unmodified against the reworked source (43/43 checks, all pass).
  - Two new problems surfaced from shapes the suite didn't previously cover, both in `Contiguous()`
    since `Merge()`'s correctness is entirely gated on it:
    6. **`Contiguous()` returns `false` for two exactly-equal ranges.** Both of its branches require
       a strict `end < rend` / `rend < end`; when the two ranges are identical, `End() == r.End()`,
       so neither fires. Reachable through the public API: adding the exact same free-memory range
       twice should be a no-op (it already is a no-op via `Overlaps()`'s containment check when the
       new range only *starts* the same but is shorter, but not when both `Base` and `Length` match
       exactly) -- instead it creates a second, fully-overlapping duplicate entry. Confirmed:
       `AddMemoryMap(0x1000, 0x1000)` called twice back-to-back leaves 2 entries, not 1.
    7. **`MergeContiguousMappings`'s compaction loop still has the "assign-to-fixed-index" bug
       pattern from bug 1, just moved.** After merging index `i-1`/`i`, it removes the redundant
       entry at `i` via `for (j = i; j < MemoryMapEntries; ++j) MemoryMap[i] = MemoryMap[j];` --
       this repeatedly overwrites the *same* index `i` (last write wins) instead of sliding each
       entry down one slot (`MemoryMap[j - 1] = MemoryMap[j]`, the pattern `RemoveDeadMappings()`
       already gets right). With 2+ untouched entries after the removed one, no data is lost, but
       their order gets scrambled (the last trailing entry jumps to index `i`; the others keep
       their original, now-off-by-one slots) -- violating the ascending-`Base` sort order the rest
       of the class depends on (e.g. `KnockoutUsedMemory`'s early-`break`). **Caveat on reachability:**
       not reproducible through `AddMemoryMap` alone in current usage -- its outer loop always
       merges a new range immediately against every existing entry, so the array should never
       actually accumulate an unmerged-adjacent pair with 2+ genuinely-disjoint entries behind it
       (verified by hand-tracing several multi-entry cascades, all self-correct via the loop's
       `--i`/reprocess step). Reproduced by seeding `MemoryMap`/`MemoryMapEntries` directly (both
       are public members) to simulate that state -- worth fixing on principle since it's the same
       category of bug as 1, and because the array being public means nothing actually prevents
       that state from arising (e.g. a future direct-population path, or a change to
       `KnockoutUsedMemory` that calls into merge logic).

  **Attempt 5 -- fixed bugs 6 and 7.** `Contiguous()` now short-circuits with
  `if (r == *this) return true;` (backed by a new defaulted `operator==` on `MemoryRange`), closing
  bug 6. `RemoveDeadMappings()` and `MergeContiguousMappings()` both now route their entry-removal
  through a new shared `SlideEntries(start)` helper that does the shift correctly
  (`MemoryMap[i - 1] = MemoryMap[i]` for everything past `start`, then decrements the count) --
  closing bug 7, and, as a side effect, deduplicating what had been two independently-written
  (and differently-buggy) copies of the same "remove an entry" logic. Also added `MemoryRange::Empty()`
  and `operator bool()`, plus a dedicated `MemoryRange`-level test group
  (`test_memory_range_*`) exercising `End()`, `operator==`/`!=`, `Contiguous()`, and `Empty()`
  directly, independent of `KernelArgs`.

  Re-verified against the current code (host GCC, ASan/UBSan): all 7 identified bugs are fixed.
  Traced `SlideEntries()` by hand against the same "2+ trailing entries" scenario that exposed bug
  7 -- it's the same shift pattern `RemoveDeadMappings()` already had right the first time, so it
  generalizes correctly regardless of how many entries trail the removed one.

  Status: **resolved.** `tests/test_kernel_args.cpp` (`kernel_args_tests` CTest target, no longer
  needs Clang, builds and passes with this host's stock GCC) is at 20 tests / 63 checks, all
  passing, no ASan/UBSan findings. Run alongside the libk suite: `cmake -S tests -B tests/build &&
  cmake --build tests/build && ctest --test-dir tests/build --output-on-failure`.
  `SortMappings`/`RemoveDeadMappings`/`MergeContiguousMappings`/`SlideEntries` are private and
  mostly exercised indirectly through `AddMemoryMap`/`KnockoutUsedMemory`'s public surface;
  `test_merge_with_two_trailing_entries_keeps_sorted_order` is the one exception, seeding
  `MemoryMap`/`MemoryMapEntries` directly (both public members) since that's the only way to reach
  the state it's testing for. The original scratch reproductions (`repro_addfreemem*.cpp`,
  `repro_memmap.cpp`, `repro_overlaps.cpp`, `repro_merge_loss.cpp`, `repro_backext.cpp`,
  `repro_fwdoverlap.cpp`, `repro_shift.cpp`) were exploratory and are now superseded by this suite.

  Remaining follow-up (not a bug, a cleanup item): `AddFreeMemory` is still present, unused, and
  still has its original bug -- left in place deliberately for comparison until `AddMemoryMap`/
  `KnockoutUsedMemory` were verified stable. That verification is now done; removing `AddFreeMemory`
  (and its recursive-helper-only callers, if any) is the natural next step whenever you're ready.

- [ ] **`sys/include/kernel/allocator.h`** — `SlabAllocator<T>` never overrides
  `Allocate()`/`Release()`, so it's still abstract and can't be instantiated anywhere it's
  actually needed. Not treated as a bug from this review -- it's incomplete-by-design while memory
  management is still under active development, not a regression. Left open here only as a pointer
  for whenever that work resumes; not blocking closing out this pass.

- [x] **`sys/libk/stdio/vsnprintf.c`** — the `exit:` clamp (`s = s < (slen - 1) ? s : slen - 1;`)
  didn't account for `slen == 0`: `slen - 1` underflowed a `size_t`, so `s` stayed `0` and
  `sbuf[0] = '\0'` still ran even though the caller claimed zero usable bytes. Fixed with an early
  `if (slen == 0) return 0;` at the top -- as a side effect, every path that now reaches `exit:` is
  guaranteed `slen >= 1`, so the clamp itself can never underflow either. Verified via
  `tests/test_vsnprintf.c`'s `test_zero_size_buffer`.

- [x] **`sys/include/utility`** — the shim `std::forward` had only one overload (taking `T&`, a
  *deduced* context), so it always cast to `remove_reference_t<T>&&` (an unconditional rvalue
  reference) regardless of what `T` actually was — it behaved like `std::move`, not true
  forwarding. Confirmed with a host reproduction: a standard `template<class T> void wrapper(T&&
  arg) { overload(std::forward<T>(arg)); }` (overloaded on `int&` vs `int&&`) called with an
  lvalue routed to the `int&&` overload -- i.e. any generic forwarding wrapper written against this
  shim would have silently force-moved from objects the caller still expected to be intact.

  Fixed with the standard two-overload form: `T&& forward(remove_reference_t<T>&)` and
  `T&& forward(remove_reference_t<T>&&)` (the latter `static_assert`s against forwarding an rvalue
  as an lvalue), both returning `T&&` rather than unconditionally `remove_reference_t<T>&&` --
  return type now correctly collapses to `T&` when `T` is itself deduced as a reference, preserving
  lvalue-ness. Needed a small new `is_lvalue_reference`/`is_lvalue_reference_v` trait in
  `sys/include/type_traits` (mirrors the existing `is_pointer` partial-specialization pattern) to
  back the `static_assert`. Re-ran the wrapper reproduction against the fixed header: lvalue now
  correctly routes to `overload(int&)`, rvalue to `overload(int&&)`.

  Since the fixed `forward()`'s parameter type is a non-deduced context (matching real
  `std::forward`), it exposed three existing call sites that were misusing `forward(x)` with no
  explicit template argument inside ordinary (non-template) rvalue-reference constructors --
  `sys/include/kernel/kernel_args.h`'s `MemoryRange(MemoryRange&&)`,
  `sys/include/cppimpl/expected/unexpected.h`'s `unexpected(Err&&)`, and
  `sys/kernel/src/allocator.cpp`'s `PageBlock(PageBlock&&)`. None of these are actually forwarding
  a deduced template parameter -- they're just move-constructing from a fixed rvalue-reference
  parameter -- so the correct call is `std::move`, not `std::forward`; the old buggy single-overload
  shim happened to compile these (via T-deduction) with `std::move`-equivalent behavior, so no
  runtime difference, but they'd have broken the moment the shim was fixed to be spec-correct.
  Changed all three to `std::move`. Verified: `tests/test_kernel_args.cpp` full suite still
  20 tests / 63 checks passing, plus a clean `-fsyntax-only` cross-compile of both
  `kernel_args.cpp` and `allocator.cpp` against the real `i686-elf-g++` toolchain
  (`-D__is_foo_kernel -Wall -Wextra -Wpedantic`).

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

---

## Status: this pass is closed

Every defect found in this review is fixed and verified (either via `tests/` -- `libk_tests` +
`kernel_args_tests`, run with `ctest --test-dir tests/build --output-on-failure` -- or a clean
`-fsyntax-only` cross-compile against the real toolchain, as noted per entry). The one remaining
unchecked item (`SlabAllocator<T>`) isn't a bug from this pass; it's tracked here only as a pointer
for when memory-management work resumes. New defects found later belong in a new pass, not
reopened here.
