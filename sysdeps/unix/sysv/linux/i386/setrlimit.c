/* Copyright (C) 1999, 2000, 2003 Free Software Foundation, Inc.
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
#include <sys/param.h>
#include <sys/resource.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <shlib-compat.h>
#include <bp-checks.h>

#include "kernel-features.h"

extern int __new_setrlimit (enum __rlimit_resource resource,
			    const struct rlimit *__unboundedrlimits);

/* Linux 2.3.25 introduced a new system call since the types used for
   the limits are now unsigned.  */
#if defined __NR_ugetrlimit && !defined __ASSUME_NEW_GETRLIMIT_SYSCALL
extern int __have_no_new_getrlimit; /* from getrlimit.c */
#endif

int
__new_setrlimit (enum __rlimit_resource resource, const struct rlimit *rlimits)
{
#ifdef __ASSUME_NEW_GETRLIMIT_SYSCALL
  return INLINE_SYSCALL (setrlimit, 2, resource, CHECK_1 (rlimits));
#else
  struct rlimit rlimits_small;

# ifdef __NR_ugetrlimit
  if (__have_no_new_getrlimit == 0)
    {
      /* Check if the new ugetrlimit syscall exists.  We must do this
	 first because older kernels don't reject negative rlimit
	 values in setrlimit.  */
      int result = INLINE_SYSCALL (ugetrlimit, 2, resource, __ptrvalue (&rlimits_small));
      if (result != -1 || errno != ENOSYS)
	/* The syscall exists.  */
	__have_no_new_getrlimit = -1;
      else
	/* The syscall does not exist.  */
	__have_no_new_getrlimit = 1;
    }
  if (__have_no_new_getrlimit < 0)
    return INLINE_SYSCALL (setrlimit, 2, resource, CHECK_1 (rlimits));
# endif

  /* We might have to correct the limits values.  Since the old values
     were signed the new values might be too large.  */
  rlimits_small.rlim_cur = MIN ((unsigned long int) rlimits->rlim_cur,
				RLIM_INFINITY >> 1);
  rlimits_small.rlim_max = MIN ((unsigned long int) rlimits->rlim_max,
				RLIM_INFINITY >> 1);

  /* Use the adjusted values.  */
  return INLINE_SYSCALL (setrlimit, 2, resource, __ptrvalue (&rlimits_small));
#endif
}

weak_alias (__new_setrlimit, __setrlimit);
versioned_symbol (libc, __new_setrlimit, setrlimit, GLIBC_2_2);
