/* Copyright (C) 1997-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jes Sorensen, <Jes.Sorensen@cern.ch>, April 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Linux/x86_64 only has rt signals, thus we do not even want to try falling
   back to the old style signals as the default Linux handler does. */

#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* Get and/or change the set of blocked signals.  */
int
__sigprocmask (int how, const sigset_t *set, sigset_t *oset)
{

  /* XXX The size argument hopefully will have to be changed to the
     real size of the user-level sigset_t.  */
  return INLINE_SYSCALL (rt_sigprocmask, 4, how, set, oset, _NSIG / 8);
}
libc_hidden_def (__sigprocmask)
weak_alias (__sigprocmask, sigprocmask)
