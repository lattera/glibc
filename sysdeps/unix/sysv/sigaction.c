/* Copyright (C) 1992,1994,1995,1996,1997,2002 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>
#include <stddef.h>
#include <signal.h>


/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__sigaction (sig, act, oact)
     int sig;
     const struct sigaction *act;
     struct sigaction *oact;
{
  sighandler_t handler;
  int save;

  if (sig <= 0 || sig >= NSIG)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (act == NULL)
    {
      if (oact == NULL)
	return 0;
      /* Race condition, but this is the only way to do it.  */
      handler = signal (sig, SIG_IGN);
      if (handler == SIG_ERR)
	return -1;
      save = errno;
      (void) signal (sig, handler);
      __set_errno (save);
    }
  else
    {
      int i;

      if (act->sa_flags != 0)
	{
	unimplemented:
	  __set_errno (ENOSYS);
	  return -1;
	}

      for (i = 1; i < NSIG; ++i)
	if (__sigismember (&act->sa_mask, i))
	  goto unimplemented;

      handler = signal (sig, act->sa_handler);
      if (handler == SIG_ERR)
	return -1;
    }

  if (oact != NULL)
    {
      oact->sa_handler = handler;
      __sigemptyset (&oact->sa_mask);
      oact->sa_flags = 0;
    }

  return 0;
}
libc_hidden_def (__sigaction)
weak_alias (__sigaction, sigaction)
