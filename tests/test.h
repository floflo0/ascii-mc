#pragma once

#include <assert.h>
#include <check.h>

#define TEST_SUITE(name, test_cases)              \
    Suite *name##_suite(void) {                   \
        Suite *const suite = suite_create(#name); \
        assert(suite != NULL);                    \
        test_cases;                               \
        return suite;                             \
    }

#define _TEST_CASE(name, tests, fixture)             \
    {                                                \
        TCase *const test_case = tcase_create(name); \
        assert(test_case != NULL);                   \
        fixture;                                     \
        tests;                                       \
        suite_add_tcase(suite, test_case);           \
    }

#define TEST_CASE(name, tests) _TEST_CASE(name, tests, )

#define TEST_CASE_WITH_SETUP(name, tests, setup, teardown) \
    _TEST_CASE(name, tests,                                \
               tcase_add_checked_fixture(test_case, setup, teardown))

#define TEST(name) tcase_add_test(test_case, name);
