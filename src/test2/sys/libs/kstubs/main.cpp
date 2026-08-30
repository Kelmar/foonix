#include <stdio.h>
#include <string>
#include <string_view>
#include <vector>

#include "ktests.h"

struct TestCaseReg
{
    tester test_case;
    std::string description;

    constexpr TestCaseReg(tester cb, std::string_view desc) noexcept
        : test_case(cb)
        , description(desc)
    {
    }

    constexpr TestCaseReg(const TestCaseReg &) = default;
    constexpr TestCaseReg(TestCaseReg &&) = default;

    TestCaseReg &operator =(const TestCaseReg &) = default;
    TestCaseReg &operator =(TestCaseReg &&) = default;

    ~TestCaseReg() { }
};

std::vector<TestCaseReg> *g_regs = nullptr;

int g_checks = 0;
int g_failures = 0;

extern "C" void kstub_reg_test(tester test_case, const char *description)
{
    if (!g_regs)
        g_regs = new std::vector<TestCaseReg>();

    g_regs->push_back({ test_case, description });
}

extern "C" void kstub_log_test(int result, const char *condition, const char *file, int line)
{
    ++g_checks;

    if (!result)
    {
        ++g_failures;
        printf("  FAIL %s:%d: %s\n", file, line, condition);
    }
}

int main(void)
{
    if (!g_regs)
    {
        printf("No tests registered!\r\n");
        return 1;
    }

    for (auto reg : *g_regs)
    {
        printf("TEST CASE: %s\r\n", reg.description.c_str());
        reg.test_case();
    }

    printf("%d/%d checks passed.\r\n", g_checks - g_failures, g_checks);

    delete g_regs;

    return (g_failures != 0) ? 1 : 0;
}
