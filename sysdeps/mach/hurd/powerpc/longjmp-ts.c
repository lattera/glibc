/* Perform a `longjmp' on a Mach thread_state.  PowerPC version.
   Copyright (C) 1991,94,95,97,2001 Free Software Foundation, Inc.
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
  struct ppc_thread_state *ts = state;

  /* XXX should we set up the FPRs as well? And how? */
  ts->r1 = env[0].__jmpbuf[JB_GPR1];
  ts->r2 = env[0].__jmpbuf[JB_GPR2];
  ts->r14 = env[0].__jmpbuf[JB_GPRS+0];
  ts->r15 = env[0].__jmpbuf[JB_GPRS+1];
  ts->r16 = env[0].__jmpbuf[JB_GPRS+2];
  ts->r17 = env[0].__jmpbuf[JB_GPRS+3];
  ts->r18 = env[0].__jmpbuf[JB_GPRS+4];
  ts->r19 = env[0].__jmpbuf[JB_GPRS+5];
  ts->r20 = env[0].__jmpbuf[JB_GPRS+6];
  ts->r21 = env[0].__jmpbuf[JB_GPRS+7];
  ts->r22 = env[0].__jmpbuf[JB_GPRS+8];
  ts->r23 = env[0].__jmpbuf[JB_GPRS+9];
  ts->r24 = env[0].__jmpbuf[JB_GPRS+10];
  ts->r25 = env[0].__jmpbuf[JB_GPRS+11];
  ts->r26 = env[0].__jmpbuf[JB_GPRS+12];
  ts->r27 = env[0].__jmpbuf[JB_GPRS+13];
  ts->r28 = env[0].__jmpbuf[JB_GPRS+14];
  ts->r29 = env[0].__jmpbuf[JB_GPRS+15];
  ts->r30 = env[0].__jmpbuf[JB_GPRS+16];
  ts->r31 = env[0].__jmpbuf[JB_GPRS+17];
  ts->cr = env[0].__jmpbuf[JB_CR];
  ts->r3 = val ?: 1;
  ts->srr0 = ts->lr = env[0].__jmpbuf[JB_LR];
}
