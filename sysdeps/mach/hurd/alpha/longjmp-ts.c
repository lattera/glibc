/* Perform a `longjmp' on a Mach thread_state.  Alpha version.
   Copyright (C) 2002 Free Software Foundation, Inc.
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

#include <hurd/signal.h>
#include <setjmp.h>
#include <mach/thread_status.h>


/* Set up STATE to do the equivalent of `longjmp (ENV, VAL);'.  */

void
_hurd_longjmp_thread_state (void *state, jmp_buf env, int val)
{
  struct alpha_thread_state *const ts = state;

  ts->r9 = env[0].__jmpbuf[JB_S0];
  ts->r10 = env[0].__jmpbuf[JB_S1];
  ts->r11 = env[0].__jmpbuf[JB_S2];
  ts->r12 = env[0].__jmpbuf[JB_S3];
  ts->r13 = env[0].__jmpbuf[JB_S4];
  ts->r13 = env[0].__jmpbuf[JB_S5];
  ts->pc = env[0].__jmpbuf[JB_PC];
  ts->r15 = env[0].__jmpbuf[JB_FP];
  ts->r30 = env[0].__jmpbuf[JB_SP];
  ts->r0 = val ?: 1;

  /* XXX
     To mimic longjmp we ought to restore some fp registers too.
     But those registers are in struct alpha_float_state.
     The only use of this is in fork, and it probably won't matter.
  */
}
