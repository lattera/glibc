/* Copyright (C) 1992, 1994 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <signal.h>
#include <errno.h>

/* If INTERRUPT is nonzero, make signal SIG interrupt system calls
   (causing them to fail with EINTR); if INTERRUPT is zero, make system
   calls be restarted after signal SIG.  */
int
DEFUN(siginterrupt, (sig, interrupt),
      int sig AND int interrupt)
{
#ifdef	SA_RESTART
  extern sigset_t _sigintr;	/* Defined in signal.c.  */
  struct sigaction action;

  if (__sigaction (sig, (struct sigaction *) NULL, &action) < 0)
    return -1;

  if (interrupt)
    {
      __sigaddset (&_sigintr, sig);
      action.sa_flags &= ~SA_RESTART;
    }
  else
    {
      __sigdelset (&_sigintr, sig);
      action.sa_flags |= SA_RESTART;
    }

  if (__sigaction (sig, &action, (struct sigaction *) NULL) < 0)
    return -1;

  return 0;
#else
  errno = ENOSYS;
  return -1;
#endif
}
