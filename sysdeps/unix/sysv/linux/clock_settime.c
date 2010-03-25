/* Copyright (C) 2003, 2004, 2006, 2010 Free Software Foundation, Inc.
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
#include <sysdep.h>

#include "kernel-posix-cpu-timers.h"
#include <kernel-features.h>

#ifndef HAVE_CLOCK_GETRES_VSYSCALL
# undef INTERNAL_VSYSCALL
# define INTERNAL_VSYSCALL INTERNAL_SYSCALL
# undef INLINE_VSYSCALL
# define INLINE_VSYSCALL INLINE_SYSCALL
#else
# include <bits/libc-vdso.h>
#endif

#if __ASSUME_POSIX_CPU_TIMERS <= 0 && defined __NR_clock_settime
extern int __libc_missing_posix_timers attribute_hidden;
extern int __libc_missing_posix_cpu_timers attribute_hidden;

static int
maybe_syscall_settime_cpu (clockid_t clock_id, const struct timespec *tp)
{
  int e = EINVAL;

  if (!__libc_missing_posix_cpu_timers)
    {
      INTERNAL_SYSCALL_DECL (err);
      int r = INTERNAL_SYSCALL (clock_settime, err, 2, clock_id, tp);
      if (!INTERNAL_SYSCALL_ERROR_P (r, err))
	return 0;

      e = INTERNAL_SYSCALL_ERRNO (r, err);
# ifndef __ASSUME_POSIX_TIMERS
      if (e == ENOSYS)
	{
	  __libc_missing_posix_timers = 1;
	  __libc_missing_posix_cpu_timers = 1;
	  e = EINVAL;
	}
      else
# endif
	{
	  if (e == EINVAL)
	    {
	      /* Check whether the kernel supports CPU clocks at all.
		 If not, record it for the future.  */
	      r = INTERNAL_VSYSCALL (clock_getres, err, 2,
				     MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED),
				     NULL);
	      if (INTERNAL_SYSCALL_ERROR_P (r, err))
		__libc_missing_posix_cpu_timers = 1;
	    }
	}
    }

  return e;
}
#endif


#ifdef __ASSUME_POSIX_TIMERS
/* This means the REALTIME clock is definitely supported in the
   kernel.  */
# define SYSDEP_SETTIME \
  case CLOCK_REALTIME:							      \
    retval = INLINE_SYSCALL (clock_settime, 2, clock_id, tp);		      \
    break
#elif defined __NR_clock_settime
/* Is the syscall known to exist?  */
extern int __libc_missing_posix_timers attribute_hidden;

/* The REALTIME clock might be available.  Try the syscall first.  */
# define SYSDEP_SETTIME \
  case CLOCK_REALTIME:							      \
  case CLOCK_REALTIME_COARSE:						      \
    {									      \
      int e = EINVAL;							      \
									      \
      if (!__libc_missing_posix_timers)					      \
	{								      \
	  INTERNAL_SYSCALL_DECL (err);					      \
	  int r = INTERNAL_SYSCALL (clock_settime, err, 2, clock_id, tp);     \
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
	{								      \
	  __set_errno (e);						      \
	  retval = -1;							      \
	}								      \
    }									      \
    break
#endif

#ifdef __NR_clock_settime
/* We handled the REALTIME clock here.  */
# define HANDLED_REALTIME	1
#endif

#if __ASSUME_POSIX_CPU_TIMERS > 0
# define HANDLED_CPUTIME 1
# define SYSDEP_SETTIME_CPU \
  retval = INLINE_SYSCALL (clock_settime, 2, clock_id, tp)
#elif defined __NR_clock_settime
# define SYSDEP_SETTIME_CPU \
  retval = maybe_syscall_settime_cpu (clock_id, tp);			      \
  if (retval == 0)							      \
    break;								      \
  if (retval != EINVAL || !__libc_missing_posix_cpu_timers)		      \
    {									      \
      __set_errno (retval);						      \
      retval = -1;							      \
      break;								      \
    }									      \
  do { } while (0)
#endif

#include <sysdeps/unix/clock_settime.c>
