/* Validate cpu_set_t values for NPTL.  Stub version.
   Copyright (C) 2015 Free Software Foundation, Inc.
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

#include <errno.h>

/* Returns 0 if CS and SZ are valid values for the cpuset and cpuset size
   respectively.  Otherwise it returns an error number.  */
static inline int
check_cpuset_attr (const cpu_set_t *cs, const size_t sz)
{
  if (sz == 0)
    return 0;

  /* This means pthread_attr_setaffinity will return ENOSYS, which
     is the right thing when the cpu_set_t features are not available.  */
  return ENOSYS;
}
