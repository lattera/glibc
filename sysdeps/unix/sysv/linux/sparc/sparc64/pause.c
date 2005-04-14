/* pause -- suspend the process until a signal arrives.  POSIX.1 version.
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
#include <sysdep-cancel.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

/* Suspend the process until a signal arrives.
   This always returns -1 and sets errno to EINTR.  */
int
__libc_pause (void)
{
  sigset_t set;

  __sigemptyset (&set);
  INLINE_SYSCALL (rt_sigprocmask, 4, SIG_BLOCK, CHECK_SIGSET (NULL),
		  CHECK_SIGSET_NULL_OK (&set), _NSIG / 8);

  /* pause is a cancellation point, but so is sigsuspend.
     So no need for anything special here.  */

  return __sigsuspend (&set);
}
weak_alias (__libc_pause, pause)

LIBC_CANCEL_HANDLED ();		/* sigsuspend handles our cancellation.  */
