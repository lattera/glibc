/* Machine-dependent signal context structure for GNU Hurd.  MIPS version.
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

#ifndef sc_mips_thread_state

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
       structure is written to be laid out identically to:
	{
	  struct mips_thread_state ts;
	  struct mips_exc_state es;
	  struct mips_float_state fs;
	}
       trampoline.c knows this, so it must be changed if this changes.  */
#define	sc_mips_thread_state sc_gpr /* Beginning of correspondence.  */
    int sc_gpr[31];		/* "General" registers; [0] is r1.  */
    int sc_mdlo, sc_mdhi;	/* Low and high multiplication results.  */
    int sc_pc;			/* Instruction pointer.  */

    /* struct mips_exc_state */
#define sc_mips_exc_state sc_cause
    unsigned int sc_cause;	/* Machine-level trap code.  */
#define SC_CAUSE_SST	0x00000044
    unsigned int sc_badvaddr;
    unsigned int sc_coproc_used; /* Which coprocessors the thread has used.  */
#define SC_COPROC_USE_COP0	1 /* (by definition) */
#define SC_COPROC_USE_COP1	2 /* FPA */
#define	SC_COPROC_USE_FPU	SC_COPROC_USE_COP1
#define SC_COPROC_USE_COP2	4
#define SC_COPROC_USE_COP3	8

    /* struct mips_float_state
       This is only filled in if the SC_COPROC_USE_FPU bit
       is set in sc_coproc_used.  */
#define sc_mips_float_state sc_fpr
    int sc_fpr[32];		/* FP registers.  */
    int sc_fpcsr;		/* FPU status register.  */
    int sc_fpeir;		/* FP exception instruction register.  */
  };

#endif /* sc_mips_thread_state */
