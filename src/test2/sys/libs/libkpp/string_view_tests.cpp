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
 */
#include "kstubs.h"

#include <string_view>


using std::string_view;

/* ---- Construction / basic accessors ----------------------------------- */

TEST_CASE("string_view default constructor is empty()")
{
    string_view sv;

    CHECK(sv.empty());
    CHECK(sv.size() == 0);
    CHECK(sv.length() == 0);
    CHECK(sv.data() == nullptr);
}

TEST_CASE("string_view construct from literal computes length")
{
    string_view sv("hello");

    CHECK(!sv.empty());
    CHECK(sv.size() == 5);
    CHECK(sv.data()[0] == 'h');
}

TEST_CASE("string_view can construct from pointer and length")
{
    const char *raw = "hello, world";
    string_view sv(raw, 5); // deliberately shorter than the buffer

    CHECK(sv.size() == 5);
    CHECK(sv.compare(string_view("hello")) == 0);
}

TEST_CASE("string_view copy and assign operators work")
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

TEST_CASE("string_view max size returns npos as expected")
{
    string_view sv;

    CHECK(string_view::npos == static_cast<string_view::size_type>(-1));
    CHECK(sv.max_size() == string_view::npos - 1);
}

/* ---- Iteration ---------------------------------------------------------- */

TEST_CASE("string_view::begin() and end() span exactly the whole view")
{
    string_view sv("abc");

    CHECK(sv.end() - sv.begin() == 3);
    CHECK(*sv.begin() == 'a');
    CHECK(*(sv.end() - 1) == 'c');
}

TEST_CASE("string_view::begin() == end() on empty")
{
    string_view sv("");

    CHECK(sv.begin() == sv.end());
}

TEST_CASE("string_view ranged for loop visits characters in correct order")
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

TEST_CASE("string_view::front() and back() return correct characters.")
{
    string_view sv("hello");

    CHECK(sv.front() == 'h');
    CHECK(sv.back() == 'o'); // regression: back() previously returned one past the last char
}

TEST_CASE("string_view::front() and back() return same on single character string")
{
    string_view sv("x");

    CHECK(sv.front() == 'x');
    CHECK(sv.back() == 'x');
}

/* ---- at() / operator[] -------------------------------------------------- */

TEST_CASE("string_view::at() returns expected values on given positions")
{
    string_view sv("hello");

    CHECK(sv.at(0) == 'h');
    CHECK(sv.at(4) == 'o');
    CHECK(sv[1] == 'e');
}

/* ---- remove_prefix() / remove_suffix() ---------------------------------- */

TEST_CASE("string_view::remove_prefix() works as expected")
{
    string_view sv("hello");
    sv.remove_prefix(2);

    CHECK(sv.size() == 3);
    CHECK(sv.compare(string_view("llo")) == 0);
}

TEST_CASE("string_view::remove_prefix() clamps to view's length")
{
    string_view sv("hi");
    sv.remove_prefix(100);

    CHECK(sv.empty());
}

TEST_CASE("string_view::remove_suffix() works as expected")
{
    string_view sv("hello");
    sv.remove_suffix(2);

    CHECK(sv.size() == 3);
    CHECK(sv.compare(string_view("hel")) == 0);
}

TEST_CASE("string_view::remove_suffix() clamps to view's length")
{
    string_view sv("hi");
    sv.remove_suffix(100);

    CHECK(sv.empty());
}

/* ---- swap() -------------------------------------------------------------- */

TEST_CASE("string_view::swap() validate functionality")
{
    string_view a("abc");
    string_view b("de");

    a.swap(b);

    CHECK(a.compare(string_view("de")) == 0);
    CHECK(b.compare(string_view("abc")) == 0);
}

/* ---- copy() -------------------------------------------------------------- */

TEST_CASE("string_view::copy() copy's requested bytes")
{
    string_view sv("hello, world");
    char buf[16] = {};

    string_view::size_type n = sv.copy(buf, 5, 7);

    CHECK(n == 5);
    CHECK(buf[0] == 'w' && buf[4] == 'd');
}

TEST_CASE("string_view::copy() at end copies zero bytes")
{
    string_view sv("hello");
    char buf[4] = { 'X', 'X', 'X', 'X' };

    string_view::size_type n = sv.copy(buf, 4, sv.size()); // pos == size(): must not panic

    CHECK(n == 0);
    CHECK(buf[0] == 'X'); // untouched
}

TEST_CASE("string_view::copy() copies to end")
{
    string_view sv("hello");
    char buf[8] = {};

    string_view::size_type n = sv.copy(buf, string_view::npos, 2);

    CHECK(n == 3);
    CHECK(buf[0] == 'l' && buf[1] == 'l' && buf[2] == 'o');
}

/* ---- subview() / substr() ------------------------------------------------ */

TEST_CASE("string_view::substr() returns whole view with default args")
{
    string_view sv("hello");
    string_view whole = sv.substr();

    CHECK(whole.size() == 5);
    CHECK(whole.compare(sv) == 0);
}

TEST_CASE("string_view::substr() with default args on empty view does not panic")
{
    string_view sv; // default-constructed, empty
    string_view whole = sv.substr(); // regression: used to kpanic() here

    CHECK(whole.empty());
}

TEST_CASE("string_view::substr() at end is empty")
{
    string_view sv("hello");
    string_view tail = sv.substr(sv.size());

    CHECK(tail.empty());
}

TEST_CASE("string_view::substr() with explicit range returns expected result")
{
    string_view sv("hello, world");
    string_view mid = sv.substr(7, 5);

    CHECK(mid.compare(string_view("world")) == 0);
}

TEST_CASE("string_view::substr() count is trimmed to end")
{
    string_view sv("hello");
    string_view tail = sv.substr(3, 100); // count overruns the view on purpose

    CHECK(tail.compare(string_view("lo")) == 0);
}

/* ---- compare() / relational operators ------------------------------------ */

TEST_CASE("string_view::compare() equal views return 0 result")
{
    string_view a("abc");
    string_view b("abc");

    CHECK(a.compare(b) == 0);
    CHECK(a == b);
    CHECK(!(a != b));
}

TEST_CASE("string_view::compare() orders correctly on first difference")
{
    string_view a("abc");
    string_view b("abd");

    CHECK(a.compare(b) < 0);
    CHECK(a < b);
    CHECK(b > a);
}

TEST_CASE("string_view::compare() small prefix is not equal to longer string")
{
    string_view shorter("ab");
    string_view longer("abc");

    CHECK(shorter.compare(longer) != 0);
    CHECK(shorter.compare(longer) < 0);
    CHECK(!(shorter == longer));
    CHECK(shorter < longer);
}

/* ---- starts_with() -------------------------------------------------------- */

TEST_CASE("string_view::starts_with() works with character parameter")
{
    string_view sv("hello");

    CHECK(sv.starts_with('h'));
    CHECK(!sv.starts_with('e'));
    CHECK(!string_view("").starts_with('h'));
}

TEST_CASE("string_view::starts_with() works with view parameter")
{
    string_view sv("hello");

    CHECK(sv.starts_with(string_view("he")));
    CHECK(!sv.starts_with(string_view("eh")));
    CHECK(!sv.starts_with(string_view("hello, world"))); // longer than self
}

TEST_CASE("string_view::starts_with() empty view edge cases")
{
    CHECK(string_view("hello").starts_with(string_view("")));  // anything starts with ""
    CHECK(string_view("").starts_with(string_view("")));       // "" starts with ""
    CHECK(!string_view("").starts_with(string_view("x")));
}

TEST_CASE("string_view::starts_with() works with C string")
{
    string_view sv("hello");

    CHECK(sv.starts_with("he"));
    CHECK(!sv.starts_with("eh"));
    CHECK(!sv.starts_with("hello, world")); // regression: used to false-positive here
}

TEST_CASE("string_view::starts_with() regresion test scanning for '0' instead of NULL")
{
    // Regression: starts_with(const char*) used to scan for the character
    // '0' instead of the NUL terminator.
    string_view sv("a0b");

    CHECK(sv.starts_with("a0b"));
    CHECK(!sv.starts_with("a0c"));
}

TEST_CASE("string_view::starts_with() works with pointer and length")
{
    string_view sv("hello");

    CHECK(sv.starts_with("he", 2));
    CHECK(!sv.starts_with("eh", 2));
    CHECK(!sv.starts_with("hello, world", 12));
}

/* ---- ends_with() ----------------------------------------------------------- */

TEST_CASE("string_view::ends_with() works with char parameter")
{
    string_view sv("hello");

    CHECK(sv.ends_with('o'));
    CHECK(!sv.ends_with('l'));
    CHECK(!string_view("").ends_with('o'));
}

TEST_CASE("string_view::ends_with() works with view paramter")
{
    string_view sv("hello");

    CHECK(sv.ends_with(string_view("lo")));
    CHECK(!sv.ends_with(string_view("ol")));
    CHECK(!sv.ends_with(string_view("hello, world"))); // longer than self
}

TEST_CASE("string_view::ends_with() various edge cases")
{
    CHECK(string_view("hello").ends_with(string_view("")));
    CHECK(string_view("").ends_with(string_view("")));
    CHECK(!string_view("").ends_with(string_view("x")));
}

TEST_CASE("string_view::ends_with() works with C string")
{
    string_view sv("hello");

    CHECK(sv.ends_with("lo"));
    CHECK(!sv.ends_with("ol"));
    CHECK(string_view("").ends_with("")); // regression: used to short-circuit false here
}

TEST_CASE("string_view::ends_width() works with pointer and length")
{
    string_view sv("hello");

    CHECK(sv.ends_with("lo", 2));
    CHECK(!sv.ends_with("ol", 2));
}

/* ---- find() ----------------------------------------------------------------- */

TEST_CASE("string_view::find() works with character parameter")
{
    string_view sv("hello");

    CHECK(sv.find('h') == 0);
    CHECK(sv.find('l') == 2);
    CHECK(sv.find('z') == string_view::npos);
}

TEST_CASE("string_view::find() with character skips with position")
{
    string_view sv("hello");

    CHECK(sv.find('l', 3) == 3);
    CHECK(sv.find('h', 1) == string_view::npos);
    CHECK(sv.find('h', sv.size()) == string_view::npos); // pos == size(): no panic, just npos
}

TEST_CASE("string_view::find() works with string")
{
    string_view sv("hello, world");

    CHECK(sv.find(string_view("world")) == 7);
    CHECK(sv.find(string_view("xyz")) == string_view::npos);
}

TEST_CASE("string_view::find() at start works")
{
    string_view sv("hello");

    CHECK(sv.find(string_view("he")) == 0);
}

TEST_CASE("string_view::find() correctly matches exact end")
{
    string_view sv("abcdef");

    CHECK(sv.find(string_view("ef")) == 4);
}

TEST_CASE("string_view::find() handles larger needle than view")
{
    string_view sv("ab");

    CHECK(sv.find(string_view("abcdef")) == string_view::npos);
}

TEST_CASE("string_view::find() with position and count works as expected")
{
    string_view sv("hello, world");

    CHECK(sv.find("wor", 0, 3) == 7);
    CHECK(sv.find("wor", 8, 3) == string_view::npos); // starts searching past the match
}

TEST_CASE("string_view::find() with C string works")
{
    string_view sv("hello, world");

    CHECK(sv.find("world") == 7);
}

/* ---- rfind() ----------------------------------------------------------------- */

TEST_CASE("string_view::rfind() works with character parameter")
{
    string_view sv("hello");

    CHECK(sv.rfind('l') == 3);
    CHECK(sv.rfind('h') == 0);
    CHECK(sv.rfind('z') == string_view::npos);
}

TEST_CASE("string_view::rfind() skips characters with pos")
{
    string_view sv("hello");

    CHECK(sv.rfind('l', 2) == 2);
    CHECK(sv.rfind('l', 1) == string_view::npos);
}

TEST_CASE("string_view::rfind() works with other string_view")
{
    string_view sv("hello, hello");

    CHECK(sv.rfind(string_view("hello")) == 7); // rightmost occurrence
}

TEST_CASE("string_view::rfind() exact end match regression test")
{
    string_view sv("abcdef");

    CHECK(sv.rfind(string_view("ef")) == 4);
}

TEST_CASE("string_view::rfind() returns not found correctly")
{
    string_view sv("abcdef");

    CHECK(sv.rfind(string_view("xyz")) == string_view::npos);
}

TEST_CASE("string_view::rfind() handles needle large than view")
{
    string_view sv("ab");

    CHECK(sv.rfind(string_view("abcdef")) == string_view::npos);
}

TEST_CASE("string_view::rfind() works with C string")
{
    string_view sv("hello, hello");

    CHECK(sv.rfind("hello") == 7);
}

/* ---- find_first_of() / find_last_of() ----------------------------------------- */

TEST_CASE("string_view::find_first_of() with char deligates to find()")
{
    string_view sv("hello");

    CHECK(sv.find_first_of('l') == sv.find('l'));
}

TEST_CASE("string_view::find_last_of() with char deligates to rfind()")
{
    string_view sv("hello");

    CHECK(sv.find_last_of('l') == sv.rfind('l'));
}

TEST_CASE("string_view::find_first_of() matches any character")
{
    string_view sv("hello");

    CHECK(sv.find_first_of(string_view("ol")) == 2); // first 'l'
    CHECK(sv.find_first_of(string_view("xyz")) == string_view::npos);
}

TEST_CASE("string_view::find_last_of() matches any character")
{
    string_view sv("hello");

    CHECK(sv.find_last_of(string_view("lo")) == 4); // the trailing 'o'
}

TEST_CASE("string_view::find_last_of() handles not found cases")
{
    string_view sv("hello");

    CHECK(sv.find_last_of(string_view("xyz")) == string_view::npos);
}

TEST_CASE("string_view::find_first_of() works with C strings")
{
    string_view sv("hello");

    CHECK(sv.find_first_of("ol") == 2);
}

TEST_CASE("string_view::find_last_of() works with C strings")
{
    string_view sv("hello");

    CHECK(sv.find_last_of("lo") == 4);
}

/* -------------------------------------------------------------------------- */

