/* Copyright (C) 1992 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@cs.widener.edu).

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

/* Note that ANY change to this instantly implies a change to __handler.S.  */

struct sigcontext
{
  /* onsigstack flag, for the sigstack state we should restore */
  int sc_onstack;

  /* signal mask to restore */
  sigset_t sc_mask;

  /* Program counter when the signal hit.  */
  int sc_pc;

  /* registers 0 through 31 */
  int sc_regs[32];

  /* mul/div low and hi; these aren't part of a jmpbuf, but are part of the
     sigcontext and are referenced from the signal trampoline code.  */
  int sc_mdlo;
  int sc_mdhi;

  /* Flag to see if the fp's been used.  */
  int sc_ownedfp;

  /* floating point registers 0 to 31 */
  int sc_fpregs[32];
  /* control & status register for fp */
  int sc_fpc_csr;

  /* exception instruction register for fp */
  int sc_fpc_eir;

  /* The coprocessor's cause register.  */
  int sc_cause;

  /* CPU bad virtual address.  */
  int sc_badvaddr;

  /* CPU board bad physical address.  */
  int sc_badpaddr;
};

