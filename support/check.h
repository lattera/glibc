/* Functionality for reporting test results.
   Copyright (C) 2016-2017 Free Software Foundation, Inc.
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
      support_test_verify_impl (-1, __FILE__, __LINE__, #expr); \
  })

/* Record a test failure and exit if EXPR evaluates to false.  */
#define TEST_VERIFY_EXIT(expr)                                  \
  ({                                                            \
    if (expr)                                                   \
      ;                                                         \
    else                                                        \
      support_test_verify_impl (1, __FILE__, __LINE__, #expr);  \
  })

int support_print_failure_impl (const char *file, int line,
                                const char *format, ...)
  __attribute__ ((nonnull (1), format (printf, 3, 4)));
void support_exit_failure_impl (int exit_status,
                                const char *file, int line,
                                const char *format, ...)
  __attribute__ ((noreturn, nonnull (2), format (printf, 4, 5)));
void support_test_verify_impl (int status, const char *file, int line,
                               const char *expr);

/* Record a test failure.  This function returns and does not
   terminate the process.  The failure counter is stored in a shared
   memory mapping, so that failures reported in child processes are
   visible to the parent process and test driver.  This function
   depends on initialization by an ELF constructor, so it can only be
   invoked after the test driver has run.  Note that this function
   does not support reporting failures from a DSO.  */
void support_record_failure (void);

/* Internal function called by the test driver.  */
int support_report_failure (int status)
  __attribute__ ((weak, warn_unused_result));

/* Internal function used to test the failure recording framework.  */
void support_record_failure_reset (void);

__END_DECLS

#endif /* SUPPORT_CHECK_H */
