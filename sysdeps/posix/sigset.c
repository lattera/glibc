/* Copyright (C) 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#define __need_NULL
#include <stddef.h>
#include <signal.h>


/* Set the disposition for SIG.  */
__sighandler_t
sigset (sig, disp)
     int sig;
     __sighandler_t disp;
{
  struct sigaction act, oact;

#ifdef SIG_HOLD
  /* Handle SIG_HOLD first.  */
  if (disp == SIG_HOLD)
    {
      sigset_t set;

      /* Retrieve current signal set.  */
      if (__sigprocmask (SIG_SETMASK, NULL, &set) < 0)
	return SIG_ERR;

      /* Add the specified signal.  */
      if (sigaddset (&set, sig) < 0)
	return SIG_ERR;

      /* Set the new mask.  */
      if (__sigprocmask (SIG_SETMASK, &set, NULL) < 0)
	return SIG_ERR;

      return SIG_HOLD;
    }
#endif	/* SIG_HOLD */

  /* Check signal extents to protect __sigismember.  */
  if (disp == SIG_ERR || sig < 1 || sig >= NSIG)
    {
      __set_errno (EINVAL);
      return SIG_ERR;
    }

  act.sa_handler = disp;
  if (__sigemptyset (&act.sa_mask) < 0)
    return SIG_ERR;
  act.sa_flags = 0;
  if (__sigaction (sig, &act, &oact) < 0)
    return SIG_ERR;

  return oact.sa_handler;
}
