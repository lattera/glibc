/* Copyright (C) 1997, 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
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
#include <signal.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include "kernel-features.h"


extern int __syscall_sigprocmask (int, const sigset_t *__unbounded,
				  sigset_t *__unbounded);
extern int __syscall_rt_sigprocmask (int, const sigset_t *__unbounded,
				     sigset_t *__unbounded, size_t);

/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  The definition is in sigaction.c.  */
extern int __libc_missing_rt_sigs;


/* Get and/or change the set of blocked signals.  */
int
__sigprocmask (how, set, oset)
     int how;
     const sigset_t *set;
     sigset_t *oset;
{
#if __ASSUME_REALTIME_SIGNALS > 0
  return INLINE_SYSCALL (rt_sigprocmask, 4, how, CHECK_SIGSET_NULL_OK (set),
			 CHECK_SIGSET_NULL_OK (oset), _NSIG / 8);
#else
# ifdef __NR_rt_sigprocmask
  /* First try the RT signals.  */
  if (!__libc_missing_rt_sigs)
    {
      /* XXX The size argument hopefully will have to be changed to the
	 real size of the user-level sigset_t.  */
      int saved_errno = errno;
      int result = INLINE_SYSCALL (rt_sigprocmask, 4, how,
				   CHECK_SIGSET_NULL_OK (set),
				   CHECK_SIGSET_NULL_OK (oset), _NSIG / 8);

      if (result >= 0 || errno != ENOSYS)
	return result;

      __set_errno (saved_errno);
      __libc_missing_rt_sigs = 1;
    }
# endif

  return INLINE_SYSCALL (sigprocmask, 3, how, CHECK_SIGSET_NULL_OK (set),
			 CHECK_SIGSET_NULL_OK (oset));
#endif
}
weak_alias (__sigprocmask, sigprocmask)
