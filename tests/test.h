/* Minimal unit test harness for the pure game modules, built on the host by
 * the `unit-tests` devenv script. Never part of the FAP. */
#pragma once

#include <stdio.h>

static int test_failures = 0;

#define CHECK(cond)                                                                  \
    do {                                                                             \
        if(!(cond)) {                                                                \
            fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond); \
            test_failures++;                                                         \
        }                                                                            \
    } while(0)

#define CHECK_EQ(actual, expected)                                \
    do {                                                          \
        long long test_actual_ = (long long)(actual);             \
        long long test_expected_ = (long long)(expected);         \
        if(test_actual_ != test_expected_) {                      \
            fprintf(                                              \
                stderr,                                           \
                "%s:%d: CHECK_EQ(%s, %s) failed: %lld != %lld\n", \
                __FILE__,                                         \
                __LINE__,                                         \
                #actual,                                          \
                #expected,                                        \
                test_actual_,                                     \
                test_expected_);                                  \
            test_failures++;                                      \
        }                                                         \
    } while(0)

/* Runs one test function and reports it by name. */
#define RUN_TEST(fn)                                                                      \
    do {                                                                                  \
        int test_before_ = test_failures;                                                 \
        fn();                                                                             \
        fprintf(stderr, "%s %s\n", test_failures == test_before_ ? "ok  " : "FAIL", #fn); \
    } while(0)

/* Exit status for main(): 0 when every check passed. */
static inline int test_report(void) {
    if(test_failures) {
        fprintf(stderr, "%d check(s) failed\n", test_failures);
        return 1;
    }
    return 0;
}
