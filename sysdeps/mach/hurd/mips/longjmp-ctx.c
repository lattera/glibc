/* Perform a `longjmp' on a `struct sigcontext'.  MIPS version.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#include <setjmp.h>
#include <hurd/signal.h>
#include <string.h>

void
_hurd_longjmp_sigcontext (struct sigcontext *scp, jmp_buf env, int retval)
{
  scp->sc_gpr[16] = env[0].__regs[0];
  scp->sc_gpr[17] = env[0].__regs[1];
  scp->sc_gpr[18] = env[0].__regs[2];
  scp->sc_gpr[19] = env[0].__regs[3];
  scp->sc_gpr[20] = env[0].__regs[4];
  scp->sc_gpr[21] = env[0].__regs[5];
  scp->sc_gpr[22] = env[0].__regs[6];
  scp->sc_gpr[23] = env[0].__regs[7];

  scp->sc_gpr[28] = (int) env[0].__gp;
  scp->sc_fp = (int) env[0].__fp;
  scp->sc_sp = (int) env[0].__sp;
  scp->sc_pc = (int) env[0].__pc;
  scp->sc_gpr[2] = retval ?: 1;
}
