/* Copyright (C) 2003 Free Software Foundation, Inc.
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

#include <sysdep.h>

#include "kernel-features.h"


#ifdef __ASSUME_POSIX_TIMERS
/* This means the REALTIME and MONOTONIC clock are definitely
   supported in the kernel.  */
# define SYSDEP_GETTIME \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
    retval = INLINE_SYSCALL (clock_gettime, 2, clock_id, tp);		      \
    break
#elif defined __NR_clock_gettime
/* Is the syscall known to exist?  */
int __libc_missing_posix_timers attribute_hidden;

/* The REALTIME and MONOTONIC clock might be available.  Try the
   syscall first.  */
# define SYSDEP_GETTIME \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
    {									      \
      int e = EINVAL;							      \
									      \
      if (!__libc_missing_posix_timers)					      \
	{								      \
	  INTERNAL_SYSCALL_DECL (err);					      \
	  int r = INTERNAL_SYSCALL (clock_gettime, err, 2, clock_id, tp);     \
	  if (!INTERNAL_SYSCALL_ERROR_P (r, err))			      \
	    {								      \
	      retval = 0;						      \
	      break;							      \
	    }								      \
									      \
	  e = INTERNAL_SYSCALL_ERRNO (r, err);				      \
	  if (e == ENOSYS)						      \
	    {								      \
	      __libc_missing_posix_timers = 1;				      \
	      e = EINVAL;						      \
	    }								      \
	}								      \
									      \
      /* Fallback code.  */						      \
      if (e == EINVAL && clock_id == CLOCK_REALTIME)			      \
	HANDLE_REALTIME;						      \
      else								      \
	__set_errno (e);						      \
    }									      \
    break
#endif

#ifdef __NR_clock_gettime
/* We handled the REALTIME clock here.  */
# define HANDLED_REALTIME	1
#endif

#include <sysdeps/unix/clock_gettime.c>
