/* Machine-dependent signal context structure for GNU Hurd.  PowerPC version.
   Copyright (C) 2001,02 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#if !defined _SIGNAL_H && !defined _SYS_UCONTEXT_H
# error "Never use <bits/sigcontext.h> directly; include <signal.h> instead."
#endif

#ifndef sc_pc

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
	 struct ppc_thread_state basic;
	 struct ppc_exc_state exc;
	 struct ppc_float_state fpu;
       }
       trampoline.c knows this, so it must be changed if this changes.  */

#define sc_ppc_thread_state sc_srr0 /* Beginning of correspondence.  */
#define sc_pc sc_srr0 /* For sysdeps/generic/profil-counter.h.  */
    unsigned int sc_srr0;
    unsigned int sc_srr1;
    unsigned int sc_gprs[32];
    unsigned int sc_cr;
    unsigned int sc_xer;
    unsigned int sc_lr;
    unsigned int sc_ctr;
    unsigned int sc_mq;
    unsigned int sc_ts_pad;

#define sc_ppc_exc_state sc_dar
    unsigned int sc_dar;
    unsigned int sc_dsisr;
    unsigned int sc_exception;
    unsigned int sc_es_pad0;
    unsigned int sc_es_pad1[4];

#define sc_ppc_float_state sc_fprs[0]
    double sc_fprs[32];
    unsigned int sc_fpscr_pad;
    unsigned int sc_fpscr;
  };

#endif /* sc_pc */
