/* Copyright (C) 1991,1995,1996,1997,2002,2004 Free Software Foundation, Inc.
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
  struct sigvec vec, ovec;

  if (sig <= 0 || sig >= NSIG)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (act != NULL)
    {
      vec.sv_mask = act->sa_mask;
      vec.sv_handler = act->sa_handler;
      vec.sv_flags = (((act->sa_flags & SA_ONSTACK) ? SV_ONSTACK : 0) |
		      (!(act->sa_flags & SA_RESTART) ? SV_INTERRUPT : 0));
    }

  if (__sigvec(sig, act != NULL ? &vec : (struct sigvec *) NULL, &ovec) < 0)
    return -1;

  if (oact != NULL)
    {
      oact->sa_handler = (void (*) (int)) ovec.sv_handler;
      oact->sa_mask = ovec.sv_mask;
      oact->sa_flags = (((ovec.sv_flags & SV_ONSTACK) ? SA_ONSTACK : 0) |
			(!(ovec.sv_flags & SV_INTERRUPT) ? SA_RESTART : 0));
    }

  return 0;
}
libc_hidden_def (__sigaction)
weak_alias (__sigaction, sigaction)
