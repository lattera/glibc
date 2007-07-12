/* Copyright (C) 1999, 2000, 2003, 2006 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sys/resource.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <shlib-compat.h>
#include <bp-checks.h>

#include <kernel-features.h>

extern int __new_getrlimit (enum __rlimit_resource resource,
			    struct rlimit *__unbounded rlimits);


/* Linux 2.3.25 introduced a new system call since the types used for
   the limits are now unsigned.  */
#if defined __NR_ugetrlimit && !defined __ASSUME_NEW_GETRLIMIT_SYSCALL
int __have_no_new_getrlimit;
#endif

int
__new_getrlimit (enum __rlimit_resource resource, struct rlimit *rlimits)
{
#ifdef __ASSUME_NEW_GETRLIMIT_SYSCALL
  return INLINE_SYSCALL (ugetrlimit, 2, resource, CHECK_1 (rlimits));
#else
  int result;

# ifdef __NR_ugetrlimit
  if (__have_no_new_getrlimit <= 0)
    {
      result = INLINE_SYSCALL (ugetrlimit, 2, resource, CHECK_1 (rlimits));

      /* If the system call is available remember this fact and return.  */
      if (result != -1 || errno != ENOSYS)
	{
	  __have_no_new_getrlimit = -1;
	  return result;
	}

      /* Remember that the system call is not available.  */
      __have_no_new_getrlimit = 1;
    }
# endif

  /* Fall back to the old system call.  */
  result = INLINE_SYSCALL (getrlimit, 2, resource, CHECK_1 (rlimits));

  if (result == -1)
    return result;

  /* We might have to correct the limits values.  Since the old values
     were signed the infinity value is too small.  */
  if (rlimits->rlim_cur == RLIM_INFINITY >> 1)
    rlimits->rlim_cur = RLIM_INFINITY;
  if (rlimits->rlim_max == RLIM_INFINITY >> 1)
    rlimits->rlim_max = RLIM_INFINITY;

  return result;
#endif
}

weak_alias (__new_getrlimit, __getrlimit);
versioned_symbol (libc, __new_getrlimit, getrlimit, GLIBC_2_2);
