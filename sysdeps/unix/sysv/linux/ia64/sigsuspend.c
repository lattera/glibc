/* Copyright (C) 1996, 1997, 1998, 1999, 2000, 2002, 2003
   Free Software Foundation, Inc.
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
#include <sys/syscall.h>
#include <bp-checks.h>

/* Change the set of blocked signals to SET,
   wait until a signal arrives, and restore the set of blocked signals.  */
int
__sigsuspend (set)
     const sigset_t *set;
{
  /* XXX The size argument hopefully will have to be changed to the
     real size of the user-level sigset_t.  */
  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (rt_sigsuspend, 2, CHECK_SIGSET (set), _NSIG / 8);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = INLINE_SYSCALL (rt_sigsuspend, 2, CHECK_SIGSET (set), _NSIG / 8);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
libc_hidden_def (__sigsuspend)
weak_alias (__sigsuspend, sigsuspend)
strong_alias (__sigsuspend, __libc_sigsuspend)
