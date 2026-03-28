#ifndef MSLANGC_TEST_ASSERT_H_
#define MSLANGC_TEST_ASSERT_H_

#include <stdio.h>

#define TEST_ASSERT(expr)                                                  \
  do {                                                                     \
    if (!(expr)) {                                                         \
      fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, \
              #expr);                                                      \
      return 1;                                                            \
    }                                                                      \
  } while (0)

#endif