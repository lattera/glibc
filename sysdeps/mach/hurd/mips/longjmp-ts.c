/* Perform a `longjmp' on a Mach thread_state.  MIPS version.
Copyright (C) 1991, 1994 Free Software Foundation, Inc.
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

#include <hurd/signal.h>
#include <setjmp.h>
#include <mach/thread_status.h>


/* Set up STATE to do the equivalent of `longjmp (ENV, VAL);'.  */

void
_hurd_longjmp_thread_state (void *state, jmp_buf env, int val)
{
  struct mips_thread_state *ts = state;

  ts->r16 = env[0].__regs[0];
  ts->r17 = env[0].__regs[1];
  ts->r18 = env[0].__regs[2];
  ts->r19 = env[0].__regs[3];
  ts->r20 = env[0].__regs[4];
  ts->r21 = env[0].__regs[5];
  ts->r22 = env[0].__regs[6];
  ts->r23 = env[0].__regs[7];
  ts->r28 = (int) env[0].__gp;
  ts->r29 = (int) env[0].__sp;
  ts->r30 = (int) env[0].__fp;
  ts->pc = (int) env[0].__pc;
  ts->r2 = val ?: 1;
}
