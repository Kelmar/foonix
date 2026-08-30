#ifndef TEST_KTESTS_H__
#define TEST_KTESTS_H__

typedef void (*tester)(void);

#ifdef __cplusplus
extern "C" {
#endif

void kstub_reg_test(tester test_case, const char *description);

void kstub_log_test(int result, const char *condition, const char *file, int line);

#ifdef __cplusplus
}
#endif

#define KSTUB_CAT_IN(A__, B__) A__ ## B__
#define KSTUB_CAT(A__, B__) KSTUB_CAT_IN(A__, B__)

#define KSTUB_ANON(X__) KSTUB_CAT(X__, __COUNTER__)

#define KSTUB_ONSTART __attribute__((constructor))

#define KSTUB_REGISTER_FUN(NAME__, DESC__) \
    KSTUB_ONSTART static void NAME__ ## _REG() { kstub_reg_test(NAME__, DESC__); }

#define KSTUB_BUILD_AND_REG(NAME__, DESC__) \
    static void NAME__ (void); \
    KSTUB_REGISTER_FUN(NAME__, DESC__) \
    static void NAME__ (void)

#define TEST_CASE(DESC__) KSTUB_BUILD_AND_REG(KSTUB_ANON(TEST_FUNC_), DESC__)

#define CHECK(COND__) kstub_log_test(COND__, #COND__, __FILE__, __LINE__)

#define CHECK_MEM_EQ(VAL__, EXP__, SZ__) CHECK(memcmp(VAL__, EXP__, SZ__) == 0)
#define CHECK_STR_EQ(VAL__, EXP__) CHECK(strcmp(VAL__, EXP__) == 0)

#endif /* TEST_KTESTS_H__ */

