/* Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <sys/param.h>
#include <sys/resource.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include "kernel-features.h"


/* Linux 2.3.25 introduced a new system call since the types used for
   the limits are now unsigned.  */
#if !defined __ASSUME_NEW_GETRLIMIT_SYSCALL && defined __NR_ugetrlimit
static int no_new_getrlimit;
#else
# define no_new_getrlimit	0
#endif

int
__setrlimit (resource, rlimits)
     enum __rlimit_resource resource;
     const struct rlimit *rlimits;
{
#ifdef __NR_ugetrlimit
  if (! no_new_getrlimit)
    {
      int result = INLINE_SYSCALL (setrlimit, 2, resource, rlimits);

# ifndef __ASSUME_NEW_GETRLIMIT_SYSCALL
      /* If the system call is available return.  */
      if (result != -1 || errno != ENOSYS)
# endif
	return result;

# ifndef __ASSUME_NEW_GETRLIMIT_SYSCALL
      /* Remember that the system call is not available.  */
      no_new_getrlimit = 1;
# endif
    }
#endif

#ifndef __ASSUME_NEW_GETRLIMIT_SYSCALL
  /* We might have to correct the limits values.  Since the old values
     were signed the new values are too large.  */
  rlimits->rlim_cur = MIN ((unsigned long int) rlimits->rlim_cur,
			   RLIM_INFINITY >> 2);
  rlimits->rlim_max = MIN ((unsigned long int) rlimits->rlim_max,
			   RLIM_INFINITY >> 2);

  /* Fall back on the old system call.  */
  return INLINE_SYSCALL (setrlimit, 2, resource, rlimits);
#endif
}
weak_alias (__setrlimit, setrlimit)
