/* Validate cpu_set_t values for NPTL.  Linux version.
   Copyright (C) 2002-2015 Free Software Foundation, Inc.
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

#include <pthread.h>
#include <errno.h>


/* Defined in pthread_setaffinity.c.  */
extern size_t __kernel_cpumask_size attribute_hidden;
extern int __determine_cpumask_size (pid_t tid);

/* Returns 0 if CS and SZ are valid values for the cpuset and cpuset size
   respectively.  Otherwise it returns an error number.  */
static inline int
check_cpuset_attr (const cpu_set_t *cs, const size_t sz)
{
  if (__kernel_cpumask_size == 0)
    {
      int res = __determine_cpumask_size (THREAD_SELF->tid);
      if (res)
	return res;
    }

  /* Check whether the new bitmask has any bit set beyond the
     last one the kernel accepts.  */
  for (size_t cnt = __kernel_cpumask_size; cnt < sz; ++cnt)
    if (((char *) cs)[cnt] != '\0')
      /* Found a nonzero byte.  This means the user request cannot be
	 fulfilled.  */
      return EINVAL;

  return 0;
}
