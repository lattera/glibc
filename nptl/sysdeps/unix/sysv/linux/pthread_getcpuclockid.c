/* pthread_getcpuclockid -- Get POSIX clockid_t for a pthread_t.  Linux version
   Copyright (C) 2000,2001,2002,2003,2004,2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <pthreadP.h>
#include <sys/time.h>
#include <tls.h>
#include <kernel-features.h>
#include <kernel-posix-cpu-timers.h>


#if !(__ASSUME_POSIX_CPU_TIMERS > 0)
int __libc_missing_posix_cpu_timers attribute_hidden;
#endif
#if !(__ASSUME_POSIX_TIMERS > 0)
int __libc_missing_posix_timers attribute_hidden;
#endif

int
pthread_getcpuclockid (threadid, clockid)
     pthread_t threadid;
     clockid_t *clockid;
{
  struct pthread *pd = (struct pthread *) threadid;

  /* Make sure the descriptor is valid.  */
  if (INVALID_TD_P (pd))
    /* Not a valid thread handle.  */
    return ESRCH;

#ifdef __NR_clock_getres
  /* The clockid_t value is a simple computation from the TID.
     But we do a clock_getres call to validate it if we aren't
     yet sure we have the kernel support.  */

  const clockid_t tidclock = MAKE_THREAD_CPUCLOCK (pd->tid, CPUCLOCK_SCHED);

# if !(__ASSUME_POSIX_CPU_TIMERS > 0)
#  if !(__ASSUME_POSIX_TIMERS > 0)
  if (__libc_missing_posix_timers && !__libc_missing_posix_cpu_timers)
    __libc_missing_posix_cpu_timers = 1;
#  endif
  if (!__libc_missing_posix_cpu_timers)
    {
      INTERNAL_SYSCALL_DECL (err);
      int r = INTERNAL_SYSCALL (clock_getres, err, 2, tidclock, NULL);
      if (!INTERNAL_SYSCALL_ERROR_P (r, err))
# endif
	{
	  *clockid = tidclock;
	  return 0;
	}

# if !(__ASSUME_POSIX_CPU_TIMERS > 0)
#  if !(__ASSUME_POSIX_TIMERS > 0)
      if (INTERNAL_SYSCALL_ERRNO (r, err) == ENOSYS)
	{
	  /* The kernel doesn't support these calls at all.  */
	  __libc_missing_posix_timers = 1;
	  __libc_missing_posix_cpu_timers = 1;
	}
      else
#  endif
	if (INTERNAL_SYSCALL_ERRNO (r, err) == EINVAL)
	  {
	    /* The kernel doesn't support these clocks at all.  */
	    __libc_missing_posix_cpu_timers = 1;
	  }
      else
	return INTERNAL_SYSCALL_ERRNO (r, err);
    }
# endif
#endif

#ifdef CLOCK_THREAD_CPUTIME_ID
  /* We need to store the thread ID in the CLOCKID variable together
     with a number identifying the clock.  We reserve the low 3 bits
     for the clock ID and the rest for the thread ID.  This is
     problematic if the thread ID is too large.  But 29 bits should be
     fine.

     If some day more clock IDs are needed the ID part can be
     enlarged.  The IDs are entirely internal.  */
  if (pd->tid >= 1 << (8 * sizeof (*clockid) - CLOCK_IDFIELD_SIZE))
    return ERANGE;

  /* Store the number.  */
  *clockid = CLOCK_THREAD_CPUTIME_ID | (pd->tid << CLOCK_IDFIELD_SIZE);

  return 0;
#else
  /* We don't have a timer for that.  */
  return ENOENT;
#endif
}
