/*
 * Native host tests for <string_view> (sys/include/string_view), compiled
 * and linked against the real header rather than a hand-copied
 * reproduction. Runs with ASan/UBSan (see tests/CMakeLists.txt) against the
 * real libk memcmp/memchr/strlen/memcpy backing the header, so this catches
 * genuine out-of-bounds reads, not just wrong-answer logic bugs.
 *
 * This project's basic_string_view isn't a full C++23 implementation (no
 * char_traits, no <=>, etc.) -- these tests cover the subset that exists,
 * plus the non-standard two-arg starts_with()/ends_with()/find_*_of()
 * extensions.
 *
 * One test below is a known, currently-failing case, marked as such:
 * compare()'s two-argument overload only memcmp()s over the shorter of the
 * two lengths and returns that result directly, without breaking a tie when
 * one string is a strict prefix of the other. Since every relational
 * operator is built on compare(), this means e.g. "ab" == "abc" currently
 * evaluates to true. Expected to fail until compare() breaks length ties.
 */
#include <string_view>

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

using std::string_view;

/* ---- Construction / basic accessors ----------------------------------- */

static void test_default_construct_is_empty()
{
    string_view sv;

    CHECK(sv.empty());
    CHECK(sv.size() == 0);
    CHECK(sv.length() == 0);
    CHECK(sv.data() == nullptr);
}

static void test_construct_from_literal_computes_length()
{
    string_view sv("hello");

    CHECK(!sv.empty());
    CHECK(sv.size() == 5);
    CHECK(sv.data()[0] == 'h');
}

static void test_construct_from_pointer_and_length()
{
    const char *raw = "hello, world";
    string_view sv(raw, 5); // deliberately shorter than the buffer

    CHECK(sv.size() == 5);
    CHECK(sv.compare(string_view("hello")) == 0);
}

static void test_copy_construct_and_assign()
{
    string_view a("abc");
    string_view b(a);
    string_view c;
    c = a;

    CHECK(b.data() == a.data());
    CHECK(b.size() == a.size());
    CHECK(c.data() == a.data());
    CHECK(c.size() == a.size());
}

static void test_max_size_and_npos()
{
    string_view sv;

    CHECK(string_view::npos == static_cast<string_view::size_type>(-1));
    CHECK(sv.max_size() == string_view::npos - 1);
}

/* ---- Iteration ---------------------------------------------------------- */

static void test_begin_end_span_exactly_the_view()
{
    string_view sv("abc");

    CHECK(sv.end() - sv.begin() == 3);
    CHECK(*sv.begin() == 'a');
    CHECK(*(sv.end() - 1) == 'c');
}

static void test_empty_view_begin_equals_end()
{
    string_view sv("");

    CHECK(sv.begin() == sv.end());
}

static void test_range_for_visits_each_character_in_order()
{
    string_view sv("abc");
    char collected[4] = {};
    int i = 0;

    for (char ch : sv)
        collected[i++] = ch;

    CHECK(i == 3);
    CHECK(collected[0] == 'a' && collected[1] == 'b' && collected[2] == 'c');
}

/* ---- front() / back() -------------------------------------------------- */

static void test_front_and_back()
{
    string_view sv("hello");

    CHECK(sv.front() == 'h');
    CHECK(sv.back() == 'o'); // regression: back() previously returned one past the last char
}

static void test_single_char_front_equals_back()
{
    string_view sv("x");

    CHECK(sv.front() == 'x');
    CHECK(sv.back() == 'x');
}

/* ---- at() / operator[] -------------------------------------------------- */

static void test_at_and_index_operator()
{
    string_view sv("hello");

    CHECK(sv.at(0) == 'h');
    CHECK(sv.at(4) == 'o');
    CHECK(sv[1] == 'e');
}

/* ---- remove_prefix() / remove_suffix() ---------------------------------- */

static void test_remove_prefix_basic()
{
    string_view sv("hello");
    sv.remove_prefix(2);

    CHECK(sv.size() == 3);
    CHECK(sv.compare(string_view("llo")) == 0);
}

static void test_remove_prefix_clamps_to_length()
{
    string_view sv("hi");
    sv.remove_prefix(100);

    CHECK(sv.empty());
}

static void test_remove_suffix_basic()
{
    string_view sv("hello");
    sv.remove_suffix(2);

    CHECK(sv.size() == 3);
    CHECK(sv.compare(string_view("hel")) == 0);
}

static void test_remove_suffix_clamps_to_length()
{
    string_view sv("hi");
    sv.remove_suffix(100);

    CHECK(sv.empty());
}

/* ---- swap() -------------------------------------------------------------- */

static void test_member_swap_exchanges_views()
{
    string_view a("abc");
    string_view b("de");

    a.swap(b);

    CHECK(a.compare(string_view("de")) == 0);
    CHECK(b.compare(string_view("abc")) == 0);
}

/* ---- copy() -------------------------------------------------------------- */

static void test_copy_into_buffer()
{
    string_view sv("hello, world");
    char buf[16] = {};

    string_view::size_type n = sv.copy(buf, 5, 7);

    CHECK(n == 5);
    CHECK(buf[0] == 'w' && buf[4] == 'd');
}

static void test_copy_at_exact_end_copies_zero()
{
    string_view sv("hello");
    char buf[4] = { 'X', 'X', 'X', 'X' };

    string_view::size_type n = sv.copy(buf, 4, sv.size()); // pos == size(): must not panic

    CHECK(n == 0);
    CHECK(buf[0] == 'X'); // untouched
}

static void test_copy_default_count_copies_to_end()
{
    string_view sv("hello");
    char buf[8] = {};

    string_view::size_type n = sv.copy(buf, string_view::npos, 2);

    CHECK(n == 3);
    CHECK(buf[0] == 'l' && buf[1] == 'l' && buf[2] == 'o');
}

/* ---- subview() / substr() ------------------------------------------------ */

static void test_substr_default_args_returns_whole_view()
{
    string_view sv("hello");
    string_view whole = sv.substr();

    CHECK(whole.size() == 5);
    CHECK(whole.compare(sv) == 0);
}

static void test_substr_default_args_on_empty_view_does_not_panic()
{
    string_view sv; // default-constructed, empty
    string_view whole = sv.substr(); // regression: used to kpanic() here

    CHECK(whole.empty());
}

static void test_substr_at_exact_end_is_empty()
{
    string_view sv("hello");
    string_view tail = sv.substr(sv.size());

    CHECK(tail.empty());
}

static void test_substr_with_explicit_range()
{
    string_view sv("hello, world");
    string_view mid = sv.substr(7, 5);

    CHECK(mid.compare(string_view("world")) == 0);
}

static void test_substr_count_is_trimmed_at_end()
{
    string_view sv("hello");
    string_view tail = sv.substr(3, 100); // count overruns the view on purpose

    CHECK(tail.compare(string_view("lo")) == 0);
}

/* ---- compare() / relational operators ------------------------------------ */

static void test_compare_equal_views()
{
    string_view a("abc");
    string_view b("abc");

    CHECK(a.compare(b) == 0);
    CHECK(a == b);
    CHECK(!(a != b));
}

static void test_compare_orders_by_first_difference()
{
    string_view a("abc");
    string_view b("abd");

    CHECK(a.compare(b) < 0);
    CHECK(a < b);
    CHECK(b > a);
}

/*
 * Known bug, not yet fixed: compare()'s two-arg overload only memcmp()s the
 * shared prefix and returns that result directly -- it never breaks a tie
 * by length when one string is a strict prefix of the other. So "ab" and
 * "abc" compare equal, which is wrong (and makes operator== wrong too).
 * Expected to fail until compare() adds a length tiebreak after a zero
 * memcmp() result.
 */
static void test_compare_prefix_is_not_equal()
{
    string_view shorter("ab");
    string_view longer("abc");

    CHECK(shorter.compare(longer) != 0);
    CHECK(shorter.compare(longer) < 0);
    CHECK(!(shorter == longer));
    CHECK(shorter < longer);
}

/* ---- starts_with() -------------------------------------------------------- */

static void test_starts_with_char()
{
    string_view sv("hello");

    CHECK(sv.starts_with('h'));
    CHECK(!sv.starts_with('e'));
    CHECK(!string_view("").starts_with('h'));
}

static void test_starts_with_view()
{
    string_view sv("hello");

    CHECK(sv.starts_with(string_view("he")));
    CHECK(!sv.starts_with(string_view("eh")));
    CHECK(!sv.starts_with(string_view("hello, world"))); // longer than self
}

static void test_starts_with_view_empty_edge_cases()
{
    CHECK(string_view("hello").starts_with(string_view("")));  // anything starts with ""
    CHECK(string_view("").starts_with(string_view("")));       // "" starts with ""
    CHECK(!string_view("").starts_with(string_view("x")));
}

static void test_starts_with_c_string()
{
    string_view sv("hello");

    CHECK(sv.starts_with("he"));
    CHECK(!sv.starts_with("eh"));
    CHECK(!sv.starts_with("hello, world")); // regression: used to false-positive here
}

static void test_starts_with_c_string_containing_digit_zero()
{
    // Regression: starts_with(const char*) used to scan for the character
    // '0' instead of the NUL terminator.
    string_view sv("a0b");

    CHECK(sv.starts_with("a0b"));
    CHECK(!sv.starts_with("a0c"));
}

static void test_starts_with_pointer_and_length()
{
    string_view sv("hello");

    CHECK(sv.starts_with("he", 2));
    CHECK(!sv.starts_with("eh", 2));
    CHECK(!sv.starts_with("hello, world", 12));
}

/* ---- ends_with() ----------------------------------------------------------- */

static void test_ends_with_char()
{
    string_view sv("hello");

    CHECK(sv.ends_with('o'));
    CHECK(!sv.ends_with('l'));
    CHECK(!string_view("").ends_with('o'));
}

static void test_ends_with_view()
{
    string_view sv("hello");

    CHECK(sv.ends_with(string_view("lo")));
    CHECK(!sv.ends_with(string_view("ol")));
    CHECK(!sv.ends_with(string_view("hello, world"))); // longer than self
}

static void test_ends_with_view_empty_edge_cases()
{
    CHECK(string_view("hello").ends_with(string_view("")));
    CHECK(string_view("").ends_with(string_view("")));
    CHECK(!string_view("").ends_with(string_view("x")));
}

static void test_ends_with_c_string()
{
    string_view sv("hello");

    CHECK(sv.ends_with("lo"));
    CHECK(!sv.ends_with("ol"));
    CHECK(string_view("").ends_with("")); // regression: used to short-circuit false here
}

static void test_ends_with_pointer_and_length()
{
    string_view sv("hello");

    CHECK(sv.ends_with("lo", 2));
    CHECK(!sv.ends_with("ol", 2));
}

/* ---- find() ----------------------------------------------------------------- */

static void test_find_char_basic()
{
    string_view sv("hello");

    CHECK(sv.find('h') == 0);
    CHECK(sv.find('l') == 2);
    CHECK(sv.find('z') == string_view::npos);
}

static void test_find_char_respects_pos()
{
    string_view sv("hello");

    CHECK(sv.find('l', 3) == 3);
    CHECK(sv.find('h', 1) == string_view::npos);
    CHECK(sv.find('h', sv.size()) == string_view::npos); // pos == size(): no panic, just npos
}

static void test_find_view_basic()
{
    string_view sv("hello, world");

    CHECK(sv.find(string_view("world")) == 7);
    CHECK(sv.find(string_view("xyz")) == string_view::npos);
}

static void test_find_view_at_start()
{
    string_view sv("hello");

    CHECK(sv.find(string_view("he")) == 0);
}

/*
 * Regression: find(basic_string_view) previously used a strict `<` loop
 * bound, missing a match anchored exactly at the last valid position.
 */
static void test_find_view_anchored_at_end()
{
    string_view sv("abcdef");

    CHECK(sv.find(string_view("ef")) == 4);
}

static void test_find_view_needle_longer_than_haystack()
{
    string_view sv("ab");

    CHECK(sv.find(string_view("abcdef")) == string_view::npos);
}

static void test_find_c_string_with_pos_and_count()
{
    string_view sv("hello, world");

    CHECK(sv.find("wor", 0, 3) == 7);
    CHECK(sv.find("wor", 8, 3) == string_view::npos); // starts searching past the match
}

static void test_find_c_string_default_pos()
{
    string_view sv("hello, world");

    CHECK(sv.find("world") == 7);
}

/* ---- rfind() ----------------------------------------------------------------- */

static void test_rfind_char_basic()
{
    string_view sv("hello");

    CHECK(sv.rfind('l') == 3);
    CHECK(sv.rfind('h') == 0);
    CHECK(sv.rfind('z') == string_view::npos);
}

static void test_rfind_char_respects_pos()
{
    string_view sv("hello");

    CHECK(sv.rfind('l', 2) == 2);
    CHECK(sv.rfind('l', 1) == string_view::npos);
}

static void test_rfind_view_basic()
{
    string_view sv("hello, hello");

    CHECK(sv.rfind(string_view("hello")) == 7); // rightmost occurrence
}

/*
 * Regression: rfind(basic_string_view) previously rejected any match
 * anchored exactly at the end of the view (first via a loop-invariant
 * guard checking the wrong variable, then via an off-by-one `>=`).
 */
static void test_rfind_view_anchored_at_end()
{
    string_view sv("abcdef");

    CHECK(sv.rfind(string_view("ef")) == 4);
}

static void test_rfind_view_not_found()
{
    string_view sv("abcdef");

    CHECK(sv.rfind(string_view("xyz")) == string_view::npos);
}

static void test_rfind_view_needle_longer_than_haystack()
{
    string_view sv("ab");

    CHECK(sv.rfind(string_view("abcdef")) == string_view::npos);
}

static void test_rfind_c_string_default_pos()
{
    string_view sv("hello, hello");

    CHECK(sv.rfind("hello") == 7);
}

/* ---- find_first_of() / find_last_of() ----------------------------------------- */

static void test_find_first_of_char_delegates_to_find()
{
    string_view sv("hello");

    CHECK(sv.find_first_of('l') == sv.find('l'));
}

static void test_find_last_of_char_delegates_to_rfind()
{
    string_view sv("hello");

    CHECK(sv.find_last_of('l') == sv.rfind('l'));
}

static void test_find_first_of_view_matches_any_character()
{
    string_view sv("hello");

    CHECK(sv.find_first_of(string_view("ol")) == 2); // first 'l'
    CHECK(sv.find_first_of(string_view("xyz")) == string_view::npos);
}

/*
 * Regression: find_last_of(basic_string_view)'s default position was
 * computed the same way as rfind() (m_length - v.m_length), which treats
 * it as a substring search rather than "last index containing any
 * character from v" -- under-searching by v.size() - 1 positions.
 */
static void test_find_last_of_view_matches_any_character()
{
    string_view sv("hello");

    CHECK(sv.find_last_of(string_view("lo")) == 4); // the trailing 'o'
}

static void test_find_last_of_view_not_found()
{
    string_view sv("hello");

    CHECK(sv.find_last_of(string_view("xyz")) == string_view::npos);
}

static void test_find_first_of_c_string()
{
    string_view sv("hello");

    CHECK(sv.find_first_of("ol") == 2);
}

static void test_find_last_of_c_string()
{
    string_view sv("hello");

    CHECK(sv.find_last_of("lo") == 4);
}

/* -------------------------------------------------------------------------- */

int main()
{
    test_default_construct_is_empty();
    test_construct_from_literal_computes_length();
    test_construct_from_pointer_and_length();
    test_copy_construct_and_assign();
    test_max_size_and_npos();

    test_begin_end_span_exactly_the_view();
    test_empty_view_begin_equals_end();
    test_range_for_visits_each_character_in_order();

    test_front_and_back();
    test_single_char_front_equals_back();

    test_at_and_index_operator();

    test_remove_prefix_basic();
    test_remove_prefix_clamps_to_length();
    test_remove_suffix_basic();
    test_remove_suffix_clamps_to_length();

    test_member_swap_exchanges_views();

    test_copy_into_buffer();
    test_copy_at_exact_end_copies_zero();
    test_copy_default_count_copies_to_end();

    test_substr_default_args_returns_whole_view();
    test_substr_default_args_on_empty_view_does_not_panic();
    test_substr_at_exact_end_is_empty();
    test_substr_with_explicit_range();
    test_substr_count_is_trimmed_at_end();

    test_compare_equal_views();
    test_compare_orders_by_first_difference();
    test_compare_prefix_is_not_equal();

    test_starts_with_char();
    test_starts_with_view();
    test_starts_with_view_empty_edge_cases();
    test_starts_with_c_string();
    test_starts_with_c_string_containing_digit_zero();
    test_starts_with_pointer_and_length();

    test_ends_with_char();
    test_ends_with_view();
    test_ends_with_view_empty_edge_cases();
    test_ends_with_c_string();
    test_ends_with_pointer_and_length();

    test_find_char_basic();
    test_find_char_respects_pos();
    test_find_view_basic();
    test_find_view_at_start();
    test_find_view_anchored_at_end();
    test_find_view_needle_longer_than_haystack();
    test_find_c_string_with_pos_and_count();
    test_find_c_string_default_pos();

    test_rfind_char_basic();
    test_rfind_char_respects_pos();
    test_rfind_view_basic();
    test_rfind_view_anchored_at_end();
    test_rfind_view_not_found();
    test_rfind_view_needle_longer_than_haystack();
    test_rfind_c_string_default_pos();

    test_find_first_of_char_delegates_to_find();
    test_find_last_of_char_delegates_to_rfind();
    test_find_first_of_view_matches_any_character();
    test_find_last_of_view_matches_any_character();
    test_find_last_of_view_not_found();
    test_find_first_of_c_string();
    test_find_last_of_c_string();

    test_io_printf("%d/%d checks passed\n", g_checks - g_failures, g_checks);

    return g_failures ? 1 : 0;
}
