/* Machine-dependent signal context structure for GNU Hurd.  Alpha version.
   Copyright (C) 1994,97,2001 Free Software Foundation, Inc.
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

#ifndef sc_alpha_thread_state

/* Signal handlers are actually called:
   void handler (int sig, int code, struct sigcontext *scp);  */

/* State of this thread when the signal was taken.  */
struct sigcontext
  {
    /* These first members are machine-independent.  */

    long int sc_onstack;	/* Nonzero if running on sigstack.  */
    __sigset_t sc_mask;		/* Blocked signals to restore.  */

    /* MiG reply port this thread is using.  */
    unsigned long int sc_reply_port;

    /* Port this thread is doing an interruptible RPC on.  */
    unsigned long int sc_intr_port;

    /* Error code associated with this signal (interpreted as `error_t').  */
    int sc_error;

    /* All following members are machine-dependent.  The rest of this
       structure is written to be laid out identically to:
       {
	 struct alpha_thread_state basic;
	 struct alpha_exc_state exc;
	 struct alpha_float_state fpu;
       }
       trampoline.c knows this, so it must be changed if this changes.  */

#define sc_alpha_thread_state sc_regs /* Beginning of correspondence.  */
    long int sc_regs[31];	/* General registers $0..$30.  */
    long int sc_pc;		/* Program counter.  */

    /* struct alpha_exc_state */
#define sc_alpha_exc_state sc_badvaddr
    unsigned long int sc_badvaddr;
    unsigned int sc_cause;	/* Machine-level trap code.  */
#define SC_CAUSE_SET_SSTEP	1
    int sc_used_fpa;		/* Nonzero if FPU was used.  */

    /* struct alpha_float_state
       This is only filled in if sc_used_fpa is nonzero.  */
#define sc_alpha_float_state sc_fpregs
    double sc_fpregs[31];	/* Floating point registers $f0..$f30.  */
    long int sc_fpcsr;		/* Floating point control/status register.  */
  };

#endif /* sc_alpha_thread_state */
