/* Copyright (C) 1993, 1995 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@azstarnet.com).

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sysdep.h>
#include <signal.h>

extern unsigned long __osf_sigprocmask (int how, unsigned long newmask);

int
__sigprocmask (int how, const sigset_t *set, sigset_t *oset)
{
  sigset_t setval;
  long result;

  if (set) {
    setval = *set;
  } else {
    sigemptyset(&setval);
    how = SIG_BLOCK;	/* ensure blocked mask doesn't get changed */
  }
  result = __osf_sigprocmask(how, setval);
  if (result == -1) {
    /* if there are ever more than 63 signals, we need to recode this
       in assembler since we wouldn't be able to distinguish a mask of
       all 1s from -1, but for now, we're doing just fine... */
    return result;
  }
  if (oset) {
    *oset = result;
  }
  return 0;
}

weak_alias (__sigprocmask, sigprocmask);
