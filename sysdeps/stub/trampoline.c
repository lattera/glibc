/* Set thread_state for sighandler, and sigcontext to recover.  Stub version.
Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <mach/thread_status.h>

/* Set up STATE to run a signal handler in the thread it describes.
   This should save the original state in a `struct sigcontext' on the
   thread's stack (or possibly a signal stack described by SIGALTSTACK,
   if the SA_ONSTACK bit is set in FLAGS), and return the address of
   that structure.  */

struct sigcontext *
_hurd_setup_sighandler (int flags,
			__sighandler_t handler,
			struct sigaltstack *sigaltstack,
			int signo, int sigcode,
			void *state)
{
#error "Need to write sysdeps/mach/hurd/MACHINE/trampoline.c"
}
