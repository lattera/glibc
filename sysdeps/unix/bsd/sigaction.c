/* Copyright (C) 1991, 1995 Free Software Foundation, Inc.
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
#include <sysdep.h>
#include <errno.h>
#include <stddef.h>
#include <signal.h>


/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
DEFUN(__sigaction, (sig, act, oact),
      int sig AND CONST struct sigaction *act AND struct sigaction *oact)
{
  struct sigvec vec, ovec;

  if (sig <= 0 || sig >= NSIG)
    {
      errno = EINVAL;
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
      oact->sa_handler = (void EXFUN((*), (int))) ovec.sv_handler;
      oact->sa_mask = ovec.sv_mask;
      oact->sa_flags = (((ovec.sv_flags & SV_ONSTACK) ? SA_ONSTACK : 0) |
			(!(ovec.sv_flags & SV_INTERRUPT) ? SA_RESTART : 0));
    }

  return 0;
}

weak_alias (__sigaction, sigaction)
