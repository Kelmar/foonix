/*
 * Native host tests for KernelArgs' memory-map bookkeeping
 * (sys/kernel/src/kernel_args.cpp), compiled and linked against the real
 * source file (see tests/CMakeLists.txt) rather than a hand-copied
 * reproduction.
 *
 * Several of these encode the CORRECT contract for bugs tracked in
 * BUGS.md's "AddMemoryMap / KnockoutUsedMemory / MergeContiguousMappings"
 * entry. All of those are now fixed (each such test says so, and why).
 */
#include <kernel/kernel_args.h>

#include "test_io.h"

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond) \
    do { \
        ++g_checks; \
        if (!(cond)) { \
            ++g_failures; \
            test_io_printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        } \
    } while (false)

/* ---- MemoryRange Tests ----------------------------------------------- */

static void test_memory_range_excludes_with_end()
{
    MemoryRange foo(0x1000, 0x1000); // [0x1000, 0x2000)
    paddr_t end = foo.End();

    CHECK(end == 0x1FFF); // Should exclude 0x2000
}

static void test_memory_range_operator_equals_is_correct()
{
    MemoryRange r1(0x1000, 0x1000); // [0x1000, 0x2000)
    MemoryRange r2(0x1000, 0x1000); // [0x1000, 0x2000)

    CHECK(r1 == r2);
}

static void test_memory_Range_operator_not_equals_is_correct()
{
    MemoryRange r1(0x1000, 0x1000); // [0x1000, 0x2000)
    MemoryRange r2(0x1000, 0x1001); // [0x1000, 0x2001)  // Off by one

    CHECK(r1 != r2);
}

static void test_memory_range_operator_contiguous_recognizes_same()
{
    MemoryRange r1(0x1000, 0x1000); // [0x1000, 0x2000)
    MemoryRange r2(0x1000, 0x1000); // [0x1000, 0x2000)

    CHECK(r1.Contiguous(r2));
}

static void test_memory_range_operator_contiguous_recognizes_direct()
{
    MemoryRange r1(0x1000, 0x1000); // [0x1000, 0x2000)
    MemoryRange r2(0x2000, 0x1000); // [0x2000, 0x3000)

    CHECK(r1.Contiguous(r2));

    r1.Base = 0x3000; // [0x3000, 0x4000)

    CHECK(r1.Contiguous(r2));
}

static void test_memory_range_operator_contiguous_recognizes_gaps()
{
    MemoryRange r1(0x1000, 0x1000); // [0x1000, 0x2000)
    MemoryRange r2(0x3000, 0x1000); // [0x3000, 0x4000)

    CHECK(!r1.Contiguous(r2));

    r1.Base = 0x5000; // [0x5000, 0x6000)

    CHECK(!r1.Contiguous(r2));
}

static void test_memory_range_recognizes_empty_range()
{
    MemoryRange r1(0x1000, 0); // (0x1000)

    CHECK(r1.Empty());
    CHECK(!r1);
}

static void test_memory_range_recognizes_non_empty_range()
{
    MemoryRange r1(0x1000, 0x1000); // [0x1000, 0x2000)

    CHECK(!r1.Empty());
    CHECK(r1);
}

/* ---- AddMemoryMap ---------------------------------------------------- */

static void test_add_single_range()
{
    KernelArgs ka;
    bool ok = ka.AddMemoryMap(0x1000, 0x1000); // [0x1000, 0x2000)

    CHECK(ok);
    CHECK(ka.MemoryMapEntries == 1);
    CHECK(ka.MemoryMap[0].Base == 0x1000);
    CHECK(ka.MemoryMap[0].Length == 0x1000);
}

static void test_add_two_disjoint_ranges_stay_separate()
{
    KernelArgs ka;
    ka.AddMemoryMap(0x1000, 0x1000); // [0x1000, 0x2000)
    ka.AddMemoryMap(0x5000, 0x1000); // [0x5000, 0x6000) -- a real gap, not adjacent

    CHECK(ka.MemoryMapEntries == 2);
    CHECK(ka.MemoryMap[0].Base == 0x1000);
    CHECK(ka.MemoryMap[0].End() == 0x1FFF);
    CHECK(ka.MemoryMap[1].Base == 0x5000);
    CHECK(ka.MemoryMap[1].End() == 0x5FFF);
}

static void test_add_range_fully_contained_is_noop()
{
    KernelArgs ka;
    ka.AddMemoryMap(0x1000, 0x4000); // [0x1000, 0x5000)
    bool ok = ka.AddMemoryMap(0x2000, 0x1000); // [0x2000, 0x3000) -- entirely inside

    CHECK(ok);
    CHECK(ka.MemoryMapEntries == 1);
    CHECK(ka.MemoryMap[0].Base == 0x1000);
    CHECK(ka.MemoryMap[0].End() == 0x4FFF);
}

/*
 * BUGS.md: previously failed because MergeContiguousMappings computed
 * newLength but never assigned it back to lastMapping.Length, silently
 * dropping the extension instead of growing the range. Fixed by the
 * MemoryRange::Merge()/Contiguous() rework -- now passes.
 */
static void test_add_two_touching_ranges_merge()
{
    KernelArgs ka;
    ka.AddMemoryMap(0x1000, 0x1000); // [0x1000, 0x2000)
    ka.AddMemoryMap(0x2000, 0x1000); // [0x2000, 0x3000) -- touches exactly, no gap

    CHECK(ka.MemoryMapEntries == 1);
    CHECK(ka.MemoryMap[0].Base == 0x1000);
    CHECK(ka.MemoryMap[0].End() == 0x2FFF);
}

/*
 * BUGS.md: previously failed because AddMemoryMap's backward-extend branch
 * moved mapping.Base without recomputing mapping.Length. Fixed by the
 * MemoryRange::Merge()/Contiguous() rework -- now passes.
 */
static void test_add_range_extending_backward_preserves_length()
{
    KernelArgs ka;
    ka.AddMemoryMap(0x5000, 0x1000); // [0x5000, 0x6000)
    ka.AddMemoryMap(0x1000, 0x4000); // [0x1000, 0x4FFF) -- ends right before 0x5000

    CHECK(ka.MemoryMapEntries == 1);
    CHECK(ka.MemoryMap[0].Base == 0x1000);
    CHECK(ka.MemoryMap[0].End() == 0x5FFF); // must still cover the original [.., 0x6000)
}

/*
 * Overlaps() should be symmetric: whether two ranges overlap can't depend on
 * which one you call the method on. Previously failed because the first
 * comparison was `Base < rend` (strict) instead of `Base <= rend`, missing
 * one-byte overlaps in one call direction. Fixed -- Overlaps() now uses
 * `Base <= rend`, and passes.
 */
static void test_overlaps_is_symmetric()
{
    MemoryRange a(9, 5);   // [9, 13]
    MemoryRange b(0, 10);  // [0, 9] -- shares byte 9 with a

    CHECK(a.Overlaps(b));
    CHECK(b.Overlaps(a));
}

/*
 * BUGS.md: previously failed for the same root cause as the touching-ranges
 * test above (missing newLength assignment), but exercised via a *third*
 * range bridging two already-separate entries, forcing
 * MergeContiguousMappings' own merge loop to run instead of AddMemoryMap's
 * direct adjacency branches. Fixed -- now passes.
 */
static void test_add_range_bridging_two_existing_merges_all_three()
{
    KernelArgs ka;
    ka.AddMemoryMap(0, 1000);     // [0, 999]
    ka.AddMemoryMap(2000, 1000);  // [2000, 2999] -- disjoint from the first
    ka.AddMemoryMap(1000, 1000);  // [1000, 1999] -- bridges both

    CHECK(ka.MemoryMapEntries == 1);
    CHECK(ka.MemoryMap[0].Base == 0);
    CHECK(ka.MemoryMap[0].End() == 2999); // must cover all three original ranges
}

/*
 * BUGS.md: previously failed because AddMemoryMap's Overlaps()-based
 * backward-extend branch computed `mapping.Length = maxEnd - newRange.Base;`,
 * one short under inclusive End() semantics. Fixed by MemoryRange::Merge(),
 * which correctly computes `maxEnd - minBase + 1` -- now passes.
 */
static void test_add_range_overlapping_backward_preserves_full_extent()
{
    KernelArgs ka;
    ka.AddMemoryMap(10, 11); // [10, 20]
    ka.AddMemoryMap(5, 11);  // [5, 15] -- genuinely overlaps [10,20], not just touching

    CHECK(ka.MemoryMapEntries == 1);
    CHECK(ka.MemoryMap[0].Base == 5);
    CHECK(ka.MemoryMap[0].End() == 20); // must still cover the original end
}

/*
 * BUGS.md: previously failed because Contiguous()'s two branches both
 * required a strict `<` between the two Ends, so identical ranges
 * (End() == End()) fired neither and were treated as non-mergeable. Fixed
 * by an explicit `if (r == *this) return true;` short-circuit in
 * Contiguous() -- now passes.
 */
static void test_add_identical_range_twice_is_noop()
{
    KernelArgs ka;
    ka.AddMemoryMap(0x1000, 0x1000); // [0x1000, 0x1FFF]
    ka.AddMemoryMap(0x1000, 0x1000); // exact duplicate

    CHECK(ka.MemoryMapEntries == 1);
    CHECK(ka.MemoryMap[0].Base == 0x1000);
    CHECK(ka.MemoryMap[0].End() == 0x1FFF);
}

/*
 * BUGS.md: previously failed because MergeContiguousMappings' compaction
 * loop removed the now-redundant entry at i via
 * `for (j = i; j < MemoryMapEntries; ++j) MemoryMap[i] = MemoryMap[j];`,
 * which repeatedly overwrote the SAME index i (last write wins) instead of
 * sliding each entry down by one slot -- scrambling sort order (without
 * losing data) whenever 2+ untouched entries followed the removed one.
 * Fixed by routing both RemoveDeadMappings() and MergeContiguousMappings()
 * through the new shared SlideEntries() helper, which does the shift
 * correctly. Still seeds MemoryMap/MemoryMapEntries directly (both public
 * members) since AddMemoryMap alone won't naturally produce the pre-merge
 * state this is exercising -- now passes.
 */
static void test_merge_with_two_trailing_entries_keeps_sorted_order()
{
    KernelArgs ka;
    ka.MemoryMap[0] = MemoryRange(0, 10);    // [0,9]
    ka.MemoryMap[1] = MemoryRange(10, 10);   // [10,19] -- adjacent to [0], pre-merge state
    ka.MemoryMap[2] = MemoryRange(100, 10);  // [100,109] -- untouched, disjoint
    ka.MemoryMap[3] = MemoryRange(200, 10);  // [200,209] -- untouched, disjoint
    ka.MemoryMapEntries = 4;

    // Any AddMemoryMap call triggers MergeContiguousMappings() on the whole
    // array; use a disjoint range so it takes the "add new" fallback path.
    ka.AddMemoryMap(1000, 10); // [1000,1009]

    CHECK(ka.MemoryMapEntries == 4);
    CHECK(ka.MemoryMap[0].Base == 0);
    CHECK(ka.MemoryMap[0].End() == 19);     // [0] and [1] merged
    CHECK(ka.MemoryMap[1].Base == 100);     // must stay in ascending order
    CHECK(ka.MemoryMap[2].Base == 200);
    CHECK(ka.MemoryMap[3].Base == 1000);
}

/* ---- KnockoutUsedMemory ------------------------------------------------ */

static void test_knockout_no_overlap_leaves_map_unchanged()
{
    KernelArgs ka;
    ka.KernelCode = MemoryRange(0x00100000, 0x2000);
    ka.AddMemoryMap(0x1000, 0x1000); // nowhere near the kernel

    ka.KnockoutUsedMemory();

    CHECK(ka.MemoryMapEntries == 1);
    CHECK(ka.MemoryMap[0].Base == 0x1000);
    CHECK(ka.MemoryMap[0].End() == 0x1FFF);
}

static void test_knockout_consumes_whole_mapping()
{
    KernelArgs ka;
    ka.KernelCode = MemoryRange(0x1000, 0x1000); // exactly covers the only mapping
    ka.AddMemoryMap(0x1000, 0x1000);

    ka.KnockoutUsedMemory();

    CHECK(ka.MemoryMapEntries == 0);
}

static void test_knockout_overlaps_start_of_mapping()
{
    KernelArgs ka;
    ka.KernelCode = MemoryRange(0x1000, 0x1000); // kernel = [0x1000, 0x2000)
    ka.AddMemoryMap(0x1000, 0x3000);             // mapping = [0x1000, 0x4000)

    ka.KnockoutUsedMemory();

    CHECK(ka.MemoryMapEntries == 1);
    CHECK(ka.MemoryMap[0].Base == 0x2000); // shrinks to start where the kernel ends
    CHECK(ka.MemoryMap[0].End() == 0x3FFF);
}

static void test_knockout_overlaps_end_of_mapping()
{
    KernelArgs ka;
    ka.KernelCode = MemoryRange(0x3000, 0x1000); // kernel = [0x3000, 0x4000)
    ka.AddMemoryMap(0x1000, 0x3000);             // mapping = [0x1000, 0x4000)

    ka.KnockoutUsedMemory();

    CHECK(ka.MemoryMapEntries == 1);
    CHECK(ka.MemoryMap[0].Base == 0x1000);
    CHECK(ka.MemoryMap[0].End() == 0x2FFF); // shrinks to end where the kernel starts
}

/*
 * BUGS.md: when the kernel is fully contained within one mapping, the new
 * post-kernel entry is written to index MemoryMapEntries + 1 instead of
 * MemoryMapEntries, orphaning it outside the counted range, where
 * RemoveDeadMappings() then discards it as "empty". Expected to fail until
 * fixed.
 */
static void test_knockout_kernel_fully_inside_mapping_splits_in_two()
{
    KernelArgs ka;
    ka.KernelCode = MemoryRange(0x0010'0000, 0x2000); // kernel = [0x100000, 0x102000)
    ka.AddMemoryMap(0x0000'0000, 0x0020'0000);        // mapping = [0, 0x200000)

    ka.KnockoutUsedMemory();

    CHECK(ka.MemoryMapEntries == 2);
    CHECK(ka.MemoryMap[0].Base == 0x0000'0000);
    CHECK(ka.MemoryMap[0].End() == 0x000F'FFFF);
    CHECK(ka.MemoryMap[1].Base == 0x0010'2000);
    CHECK(ka.MemoryMap[1].End() == 0x001F'FFFF);
}

int main()
{
    // MemoryRange tests
    test_memory_range_excludes_with_end();
    test_memory_range_operator_equals_is_correct();
    test_memory_Range_operator_not_equals_is_correct();
    test_memory_range_operator_contiguous_recognizes_same();
    test_memory_range_operator_contiguous_recognizes_direct();
    test_memory_range_operator_contiguous_recognizes_gaps();    
    test_memory_range_recognizes_empty_range();
    test_memory_range_recognizes_non_empty_range();

    test_add_single_range();
    test_add_two_disjoint_ranges_stay_separate();
    test_add_range_fully_contained_is_noop();
    test_add_two_touching_ranges_merge();
    test_add_range_extending_backward_preserves_length();
    test_overlaps_is_symmetric();
    test_add_range_bridging_two_existing_merges_all_three();
    test_add_range_overlapping_backward_preserves_full_extent();
    test_add_identical_range_twice_is_noop();
    test_merge_with_two_trailing_entries_keeps_sorted_order();

    test_knockout_no_overlap_leaves_map_unchanged();
    test_knockout_consumes_whole_mapping();
    test_knockout_overlaps_start_of_mapping();
    test_knockout_overlaps_end_of_mapping();
    test_knockout_kernel_fully_inside_mapping_splits_in_two();

    test_io_printf("%d/%d checks passed\n", g_checks - g_failures, g_checks);

    return g_failures ? 1 : 0;
}
