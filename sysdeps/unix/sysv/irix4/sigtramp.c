/* Copyright (C) 1992 Free Software Foundation, Inc.
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

/* The sigvec system call on MIPS Ultrix takes an additional
   parameter, which is the address that is actually called when the
   signal occurs.

   When a signal occurs, we arrange for the kernel to call __handler.
   That will save the frame and stack pointers into the context, and
   then jump to this routine.  See __handler.S.

   This code is based on sysdeps/unix/bsd/sun4/sigtramp.c, but it's
   different because since we get passed the user signal handler we
   don't actually need a trampoline.  */

#include <ansidecl.h>
#include <signal.h>
#include <stddef.h>
#include <errno.h>

/* The user's signal handler is called with three arguments.  */
typedef void (*handler_type) (int sig, int code, struct sigcontext *);

/* Defined in signal.S.  */
extern __sighandler_t EXFUN(__raw_signal, (int sig, __sighandler_t func,
				void (*)(int sig, int code,
					 struct sigcontext *,
					 handler_type)));

extern void EXFUN(__handler, (int sig, int code,
			      struct sigcontext *,
			      handler_type));

__sighandler_t
DEFUN(signal, (sig, func),
      int sig AND __sighandler_t func)
{
  return __raw_signal (sig, func, __handler);
}
