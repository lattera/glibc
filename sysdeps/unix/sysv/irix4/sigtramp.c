/* Copyright (C) 1992, 1997 Free Software Foundation, Inc.
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

/* The sigvec system call on MIPS Ultrix takes an additional
   parameter, which is the address that is actually called when the
   signal occurs.

   When a signal occurs, we arrange for the kernel to call __handler.
   That will save the frame and stack pointers into the context, and
   then jump to this routine.  See __handler.S.

   This code is based on sysdeps/unix/bsd/sun4/sigtramp.c, but it's
   different because since we get passed the user signal handler we
   don't actually need a trampoline.  */

#include <signal.h>
#include <stddef.h>
#include <errno.h>

/* The user's signal handler is called with three arguments.  */
typedef void (*handler_type) (int sig, int code, struct sigcontext *);

/* Defined in signal.S.  */
extern __sighandler_t __raw_signal __P((int sig, __sighandler_t func,
					void (*)(int sig, int code,
						 struct sigcontext *,
						 handler_type)));

extern void __handler __P((int sig, int code,
			   struct sigcontext *,
			   handler_type));

__sighandler_t
signal (sig, func)
     int sig;
     __sighandler_t func;
{
  return __raw_signal (sig, func, __handler);
}
