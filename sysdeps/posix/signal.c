/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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


sigset_t _sigintr;		/* Set by siginterrupt.  */

/* Set the handler for the signal SIG to HANDLER,
   returning the old handler, or SIG_ERR on error.  */
__sighandler_t
DEFUN(signal, (sig, handler), int sig AND __sighandler_t handler)
{
  struct sigaction act, oact;

  if (handler == SIG_ERR)
    {
      errno = EINVAL;
      return SIG_ERR;
    }

  act.sa_handler = handler;
  if (__sigemptyset (&act.sa_mask) < 0)
    return SIG_ERR;
  act.sa_flags = __sigismember (&_sigintr, sig) ? 0 : SA_RESTART;
  if (__sigaction (sig, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}
