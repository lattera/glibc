/* Machine-dependent signal context structure for GNU Hurd.  HPPA version.
   Copyright (C) 1995,97,2001 Free Software Foundation, Inc.
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

#if !defined _SIGNAL_H && !defined _SYS_UCONTEXT_H
# error "Never use <bits/sigcontext.h> directly; include <signal.h> instead."
#endif

#ifndef sc_parisc_thread_state

/* Signal handlers are actually called:
   void handler (int sig, int code, struct sigcontext *scp);  */

/* State of this thread when the signal was taken.  */
struct sigcontext
  {
    /* These first members are machine-independent.  */

    int sc_onstack;		/* Nonzero if running on sigstack.  */
    __sigset_t sc_mask;		/* Blocked signals to restore.  */

    /* MiG reply port this thread is using.  */
    unsigned int sc_reply_port;

    /* Port this thread is doing an interruptible RPC on.  */
    unsigned int sc_intr_port;

    /* Error code associated with this signal (interpreted as `error_t').  */
    int sc_error;

    /* All following members are machine-dependent.  The rest of this
       structure is written to be laid out identically to a `struct
       parisc_thread_state'.  trampoline.c knows this, so it must be
       changed if this changes.  */

#define sc_parisc_thread_state sc_flags /* Beginning of correspondence.  */
    /* "General" registers $1..$31.  */
    unsigned int sc_regs[31];

    /* Control registers.  */
    unsigned int sc_cr11;	/* sar */
    /* These four registers make up the PC.  */
    unsigned int iioq_head;
    unsigned int iisq_head;
    unsigned int iioq_tail;
    unsigned int iisq_tail;
    unsigned int sc_cr15;
    unsigned int sc_cr19;
    unsigned int sc_cr20;
    unsigned int sc_cr21;
    unsigned int sc_cr22;	/* ipsw */
    unsigned int sc_bsd_goto;	/* unused */
    unsigned int sc_sr4;
    unsigned int sc_sr0;
    unsigned int sc_sr1;
    unsigned int sc_sr2;
    unsigned int sc_sr3;
    unsigned int sc_sr5;
    unsigned int sc_sr6;
    unsigned int sc_sr7;
    unsigned int sc_cr0;
    unsigned int sc_cr8;
    unsigned int sc_cr9;
    unsigned int sc_cr10;	/* unused */
    unsigned int sc_cr12;
    unsigned int sc_cr13;
    unsigned int sc_cr24;	/* unused */
    unsigned int sc_cr25;	/* unused */
    unsigned int sc_cr26;	/* unused */
    unsigned sc_mpsfu_high;	/* unused */
    unsigned sc_mpsfu_low;	/* unused */
    unsigned sc_mpsfu_ovflo;	/* unused */
    int sc_pad;

    /* Floating point registers $f0..$f31.  */
    double sc_fpregs[32];
  };

#endif /* sc_parisc_thread_state */
