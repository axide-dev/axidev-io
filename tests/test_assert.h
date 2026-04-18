#pragma once
#ifndef AXIDEV_IO_TEST_ASSERT_H
#define AXIDEV_IO_TEST_ASSERT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_axidev_test_failures = 0;

#define TEST_CHECK(condition)                                                  \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, __LINE__,         \
              #condition);                                                     \
      ++g_axidev_test_failures;                                                \
    }                                                                          \
  } while (0)

#define TEST_CHECK_EQ_INT(actual, expected)                                    \
  do {                                                                         \
    long long _actual = (long long)(actual);                                   \
    long long _expected = (long long)(expected);                               \
    if (_actual != _expected) {                                                \
      fprintf(stderr, "%s:%d: expected %s == %s but got %lld vs %lld\n",       \
              __FILE__, __LINE__, #actual, #expected, _actual, _expected);     \
      ++g_axidev_test_failures;                                                \
    }                                                                          \
  } while (0)

#define TEST_CHECK_STR(actual, expected)                                       \
  do {                                                                         \
    const char *_actual = (actual);                                            \
    const char *_expected = (expected);                                        \
    if ((_actual == NULL && _expected != NULL) ||                              \
        (_actual != NULL && _expected == NULL) ||                              \
        (_actual != NULL && _expected != NULL &&                               \
         strcmp(_actual, _expected) != 0)) {                                   \
      fprintf(stderr, "%s:%d: expected \"%s\" but got \"%s\"\n", __FILE__,     \
              __LINE__, _expected != NULL ? _expected : "(null)",              \
              _actual != NULL ? _actual : "(null)");                           \
      ++g_axidev_test_failures;                                                \
    }                                                                          \
  } while (0)

#define TEST_RUN(function_name)                                                \
  do {                                                                         \
    fprintf(stderr, "[TEST] %s\n", #function_name);                            \
    function_name();                                                           \
  } while (0)

#endif
