/* Perform a `longjmp' on a `struct sigcontext'.  Stub version.
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

#include <setjmp.h>
#include <signal.h>
#include <string.h>

void
_hurd_longjmp_sigcontext (struct sigcontext *scp, jmp_buf env, int retval)
{
  memset (scp, 0, sizeof (*scp));
  /* Set all the registers in *SCP to the values described by ENV and RETVAL.
     After this, calling `__sigreturn (SCP)' in that thread should be just
     like calling `longjmp (ENV, RETVAL)'.  */
  #error "Need to write sysdeps/mach/hurd/MACHINE/longjmp-ctx.c"
}
