/* Perform a `longjmp' on a `struct sigcontext'.  i386 version.
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
#include <hurd/signal.h>
#include <string.h>

void
_hurd_longjmp_sigcontext (struct sigcontext *scp, jmp_buf env, int retval)
{
  memset (scp, 0, sizeof (*scp));
  scp->sc_regs[9] = env[0].__9;
  scp->sc_regs[11] = env[0].__11;
  scp->sc_regs[12] = env[0].__12;
  scp->sc_regs[13] = env[0].__13;
  scp->sc_regs[14] = env[0].__14;
  scp->sc_regs[15] = (long int) env[0].__fp;
  scp->sc_regs[30] = (long int) env[0].__sp;
  scp->sc_pc = (long int) env[0].__pc;

  memcpy (&scp->sc_fpregs[2], &env[0].__f2, sizeof (double));
}
