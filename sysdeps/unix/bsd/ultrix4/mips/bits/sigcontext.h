/* Copyright (C) 1992, 1994, 1997 Free Software Foundation, Inc.
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

#ifndef _SIGNAL_H
# error "Never use <bits/sigcontext.h> directly; include <signal.h> instead."
#endif

/* Note that ANY change to this instantly implies a change to __handler.S.  */

struct sigcontext
  {
    /* Nonzero if running on signal stack.  */
    int sc_onstack;

    /* Signal mask to restore.  */
    __sigset_t sc_mask;

    /* Program counter when the signal hit.  */
    __ptr_t sc_pc;

    /* Registers 0 through 31.  */
    int sc_regs[32];

    /* mul/div low and hi; these aren't part of a jmp_buf, but are part of the
       sigcontext and are referenced from the signal trampoline code.  */
    int sc_mdlo;
    int sc_mdhi;

    /* Flag to see if the FP's been used.  */
    int sc_ownedfp;

    /* Floating point registers 0 to 31.  */
    int sc_fpregs[32];
    /* Control & status register for FP.  */
    int sc_fpc_csr;

    /* Exception instruction register for FP. */
    int sc_fpc_eir;

    /* The coprocessor's cause register.  */
    int sc_cause;

    /* CPU bad virtual address.  */
    __ptr_t sc_badvaddr;

    /* CPU board bad physical address.  */
    __ptr_t sc_badpaddr;
  };
