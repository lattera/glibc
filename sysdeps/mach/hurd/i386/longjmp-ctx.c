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
  scp->sc_ebx = env[0].__bx;
  scp->sc_esi = env[0].__si;
  scp->sc_edi = env[0].__di;
  scp->sc_ebp = (int) env[0].__bp;
  scp->sc_uesp = (int) env[0].__sp;
  scp->sc_eip = (int) env[0].__pc;
  scp->sc_eax = retval == 0 ? 1 : retval;
}
