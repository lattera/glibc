/* Copyright (C) 1991, 1994, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <signal.h>

/* Block signals in MASK, returning the old mask.  */
int
DEFUN(__sigblock, (mask), int mask)
{
  register int sig;
  sigset_t set, oset;

  if (__sigemptyset(&set) < 0)
    return -1;
  
  if (sizeof (mask) == sizeof (set))
    *(int *) &set = mask;
  else
    for (sig = 1; sig < NSIG; ++sig)
      if ((mask & sigmask(sig)) &&
	  __sigaddset(&set, sig) < 0)
	return -1;

  if (sigprocmask(SIG_BLOCK, &set, &oset) < 0)
    return -1;

  mask = 0;
  if (sizeof (mask) == sizeof (oset))
    mask = *(int *) &oset;
  else
    for (sig = 1; sig < NSIG; ++sig)
      if (__sigismember(&oset, sig))
	mask |= sigmask(sig);

  return mask;
}

weak_alias (__sigblock, sigblock)
