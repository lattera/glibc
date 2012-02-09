/* Copyright (C) 1991, 1996, 1997, 1998, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <unistd.h>


/* Change the set of blocked signals to SET,
   wait until a signal arrives, and restore the set of blocked signals.  */
int
__sigsuspend (set)
     const sigset_t *set;
{
  int mask;
  int sig;

  if (set == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  mask = 0;
  for (sig = 1; sig <= NSIG; ++sig)
    if (__sigismember (set, sig))
      mask |= sigmask (sig);

  return __sigpause (mask, 0);
}
libc_hidden_def (__sigsuspend)
weak_alias (__sigsuspend, sigsuspend)
