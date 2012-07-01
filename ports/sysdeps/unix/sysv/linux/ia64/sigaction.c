/* Copyright (C) 1997-2000,2002,2003,2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Linux/IA64 specific sigaction
   Written by Jes Sorensen, <Jes.Sorensen@cern.ch>, April 1999.

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

/* Linux/ia64 only has rt signals, thus we do not even want to try falling
   back to the old style signals as the default Linux handler does. */

#include <errno.h>
#include <signal.h>
#include <string.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

/* The variable is shared between all wrappers around signal handling
   functions which have RT equivalents.  This is the definition.  */


/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__libc_sigaction (sig, act, oact)
     int sig;
     const struct sigaction *act;
     struct sigaction *oact;
{
  /* XXX The size argument hopefully will have to be changed to the
     real size of the user-level sigset_t.  */
  return INLINE_SYSCALL (rt_sigaction, 4, sig,
			 CHECK_1_NULL_OK (act), CHECK_1_NULL_OK (oact), _NSIG / 8);
}
libc_hidden_def (__libc_sigaction)

#ifdef WRAPPER_INCLUDE
# include WRAPPER_INCLUDE
#endif

#ifndef LIBC_SIGACTION
weak_alias (__libc_sigaction, __sigaction)
libc_hidden_def (__sigaction)
weak_alias (__libc_sigaction, sigaction)
#endif
