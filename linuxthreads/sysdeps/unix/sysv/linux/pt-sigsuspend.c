/* Internal sigsuspend system call for LinuxThreads.  Generic Linux version.
   Copyright (C) 2003 Free Software Foundation, Inc.
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
#include <signal.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <linuxthreads/internals.h>

#include "kernel-features.h"

void
__pthread_sigsuspend (const sigset_t *set)
{
  INTERNAL_SYSCALL_DECL (err);
#if !__ASSUME_REALTIME_SIGNALS
  static int __pthread_missing_rt_sigs;

# ifdef __NR_rt_sigsuspend
  /* First try the RT signals.  */
  if (!__pthread_missing_rt_sigs)
    {
      /* XXX The size argument hopefully will have to be changed to the
	 real size of the user-level sigset_t.  */
      int r;
      r = INTERNAL_SYSCALL (rt_sigsuspend, err, 2, set, _NSIG / 8);
      if (INTERNAL_SYSCALL_ERRNO (r, err) != ENOSYS)
	return;

      __pthread_missing_rt_sigs = 1;
    }
# endif

  INTERNAL_SYSCALL (sigsuspend, err, 3, 0, 0, set->__val[0]);
#else
  INTERNAL_SYSCALL (rt_sigsuspend, err, 2, set, _NSIG / 8);
#endif
}
