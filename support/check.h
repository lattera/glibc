/* Functionality for reporting test results.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef SUPPORT_CHECK_H
#define SUPPORT_CHECK_H

#include <sys/cdefs.h>

__BEGIN_DECLS

/* Record a test failure, print the failure message to standard output
   and return 1.  */
#define FAIL_RET(...) \
  return support_print_failure_impl (__FILE__, __LINE__, __VA_ARGS__)

/* Print the failure message and terminate the process with STATUS.
   Record a the process as failed if STATUS is neither EXIT_SUCCESS
   nor EXIT_UNSUPPORTED.  */
#define FAIL_EXIT(status, ...) \
  support_exit_failure_impl (status, __FILE__, __LINE__, __VA_ARGS__)

/* Record a test failure, print the failure message and terminate with
   exit status 1.  */
#define FAIL_EXIT1(...) \
  support_exit_failure_impl (1, __FILE__, __LINE__, __VA_ARGS__)

/* Print failure message and terminate with as unsupported test (exit
   status of 77).  */
#define FAIL_UNSUPPORTED(...) \
  support_exit_failure_impl (77, __FILE__, __LINE__, __VA_ARGS__)

/* Record a test failure (but continue executing) if EXPR evaluates to
   false.  */
#define TEST_VERIFY(expr)                                       \
  ({                                                            \
    if (expr)                                                   \
      ;                                                         \
    else                                                        \
      support_test_verify_impl (__FILE__, __LINE__, #expr);     \
  })

/* Record a test failure and exit if EXPR evaluates to false.  */
#define TEST_VERIFY_EXIT(expr)                                  \
  ({                                                            \
    if (expr)                                                   \
      ;                                                         \
    else                                                        \
      support_test_verify_exit_impl                             \
        (1, __FILE__, __LINE__, #expr);                         \
  })

int support_print_failure_impl (const char *file, int line,
                                const char *format, ...)
  __attribute__ ((nonnull (1), format (printf, 3, 4)));
void support_exit_failure_impl (int exit_status,
                                const char *file, int line,
                                const char *format, ...)
  __attribute__ ((noreturn, nonnull (2), format (printf, 4, 5)));
void support_test_verify_impl (const char *file, int line,
                               const char *expr);
void support_test_verify_exit_impl (int status, const char *file, int line,
                                    const char *expr)
  __attribute__ ((noreturn));

/* Record a test failure.  This function returns and does not
   terminate the process.  The failure counter is stored in a shared
   memory mapping, so that failures reported in child processes are
   visible to the parent process and test driver.  This function
   depends on initialization by an ELF constructor, so it can only be
   invoked after the test driver has run.  Note that this function
   does not support reporting failures from a DSO.  */
void support_record_failure (void);

/* Static assertion, under a common name for both C++ and C11.  */
#ifdef __cplusplus
# define support_static_assert static_assert
#else
# define support_static_assert _Static_assert
#endif

/* Compare the two integers LEFT and RIGHT and report failure if they
   are different.  */
#define TEST_COMPARE(left, right)                                       \
  ({                                                                    \
    /* + applies the integer promotions, for bitfield support.   */     \
    typedef __typeof__ (+ (left)) __left_type;                          \
    typedef __typeof__ (+ (right)) __right_type;                        \
    __left_type __left_value = (left);                                  \
    __right_type __right_value = (right);                               \
    int __left_is_positive = __left_value > 0;                          \
    int __right_is_positive = __right_value > 0;                        \
    /* Prevent use with floating-point types.  */                       \
    support_static_assert ((__left_type) 1.0 == (__left_type) 1.5,      \
                           "left value has floating-point type");       \
    support_static_assert ((__right_type) 1.0 == (__right_type) 1.5,    \
                           "right value has floating-point type");      \
    /* Prevent accidental use with larger-than-long long types.  */     \
    support_static_assert (sizeof (__left_value) <= sizeof (long long), \
                           "left value fits into long long");           \
    support_static_assert (sizeof (__right_value) <= sizeof (long long), \
                    "right value fits into long long");                 \
    /* Compare the value.  */                                           \
    if (__left_value != __right_value                                   \
        || __left_is_positive != __right_is_positive)                   \
      /* Pass the sign for printing the correct value.  */              \
      support_test_compare_failure                                      \
        (__FILE__, __LINE__,                                            \
         #left, __left_value, __left_is_positive, sizeof (__left_type), \
         #right, __right_value, __right_is_positive, sizeof (__right_type)); \
  })

/* Internal implementation of TEST_COMPARE.  LEFT_POSITIVE and
   RIGHT_POSITIVE are used to store the sign separately, so that both
   unsigned long long and long long arguments fit into LEFT_VALUE and
   RIGHT_VALUE, and the function can still print the original value.
   LEFT_SIZE and RIGHT_SIZE specify the size of the argument in bytes,
   for hexadecimal formatting.  */
void support_test_compare_failure (const char *file, int line,
                                   const char *left_expr,
                                   long long left_value,
                                   int left_positive,
                                   int left_size,
                                   const char *right_expr,
                                   long long right_value,
                                   int right_positive,
                                   int right_size);


/* Internal function called by the test driver.  */
int support_report_failure (int status)
  __attribute__ ((weak, warn_unused_result));

/* Internal function used to test the failure recording framework.  */
void support_record_failure_reset (void);

__END_DECLS

#endif /* SUPPORT_CHECK_H */
