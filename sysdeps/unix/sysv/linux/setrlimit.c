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

extern int __syscall_setrlimit (unsigned int resource,
				const struct rlimit *rlimits);
extern int __syscall_ugetrlimit (unsigned int resource,
				 const struct rlimit *rlimits);

/* Linux 2.3.25 introduced a new system call since the types used for
   the limits are now unsigned.  */
#if defined __NR_ugetrlimit && !defined __ASSUME_NEW_GETRLIMIT_SYSCALL
extern int __have_no_new_getrlimit; /* from getrlimit.c */
#endif

int
__new_setrlimit (enum __rlimit_resource resource, const struct rlimit *rlimits)
{
#ifdef __ASSUME_NEW_GETRLIMIT_SYSCALL
  return INLINE_SYSCALL (setrlimit, 2, resource, rlimits);
#else
  struct rlimit rlimits_small;

# ifdef __NR_ugetrlimit
  if (__have_no_new_getrlimit <= 0)
    {
      int result = INLINE_SYSCALL (setrlimit, 2, resource, rlimits);

      /* Return if the values are not out of range or if we positively
         know that the ugetrlimit system call exists.  */
      if (result != -1 || errno != EINVAL || __have_no_new_getrlimit < 0)
	return result;

      /* Check if the new ugetrlimit syscall exists.  */
      if (INLINE_SYSCALL (ugetrlimit, 2, resource, &rlimits_small) != -1
	  || errno != ENOSYS)
	{
	  /* There was some other error, probably RESOURCE out of range.
             Remember that the ugetrlimit system call really exists.  */
	  __have_no_new_getrlimit = -1;
	  /* Restore previous errno value.  */
	  __set_errno (EINVAL);
	  return result;
	}

      /* Remember that the kernel uses the old interface.  */
      __have_no_new_getrlimit = 1;
    }
# endif

  /* We might have to correct the limits values.  Since the old values
     were signed the new values might be too large.  */
  rlimits_small.rlim_cur = MIN ((unsigned long int) rlimits->rlim_cur,
				RLIM_INFINITY >> 1);
  rlimits_small.rlim_max = MIN ((unsigned long int) rlimits->rlim_max,
				RLIM_INFINITY >> 1);

  /* Try again with the adjusted values.  */
  return INLINE_SYSCALL (setrlimit, 2, resource, &rlimits_small);
#endif
}

#if defined PIC && DO_VERSIONING
default_symbol_version (__new_setrlimit, __setrlimit, GLIBC_2.1.3);
strong_alias (__new_setrlimit, _new_setrlimit);
default_symbol_version (_new_setrlimit, setrlimit, GLIBC_2.1.3);
#else
weak_alias (__new_setrlimit, __setrlimit);
weak_alias (__new_setrlimit, setrlimit);
#endif
