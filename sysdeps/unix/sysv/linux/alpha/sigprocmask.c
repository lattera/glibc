/* Copyright (C) 1993, 1995, 1997, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger (davidm@azstarnet.com).

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
#include <signal.h>

/* When there is kernel support for more than 64 signals, we'll have to
   switch to a new system call convention here.  */

int
__sigprocmask (int how, const sigset_t *set, sigset_t *oset)
{
  unsigned long int setval;
  long result;

  if (set)
    setval = set->__val[0];
  else
    {
      setval = 0;
      how = SIG_BLOCK;	/* ensure blocked mask doesn't get changed */
    }

  result = INLINE_SYSCALL (osf_sigprocmask, 2, how, setval);
  if (result == -1)
    /* If there are ever more than 63 signals, we need to recode this
       in assembler since we wouldn't be able to distinguish a mask of
       all 1s from -1, but for now, we're doing just fine... */
    return result;

  if (oset)
    {
      oset->__val[0] = result;
      result = _SIGSET_NWORDS;
      while (--result > 0)
	oset->__val[result] = 0;
    }
  return 0;
}

weak_alias (__sigprocmask, sigprocmask);
