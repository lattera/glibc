/* Macros for reporting test results.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

/* Print failure message to standard output and return 1.  */
#define FAIL_RET(...) \
  return support_print_failure_impl (__FILE__, __LINE__, __VA_ARGS__)

/* Print failure message and terminate the process with STATUS.  */
#define FAIL_EXIT(status, ...) \
  support_exit_failure_impl (status, __FILE__, __LINE__, __VA_ARGS__)

/* Print failure message and terminate with exit status 1.  */
#define FAIL_EXIT1(...) \
  support_exit_failure_impl (1, __FILE__, __LINE__, __VA_ARGS__)

int support_print_failure_impl (const char *file, int line,
                                const char *format, ...)
  __attribute__ ((nonnull (1), format (printf, 3, 4)));
void support_exit_failure_impl (int exit_status,
                                const char *file, int line,
                                const char *format, ...)
  __attribute__ ((noreturn, nonnull (2), format (printf, 4, 5)));


__END_DECLS

#endif /* SUPPORT_CHECK_H */
