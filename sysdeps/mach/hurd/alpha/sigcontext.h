/* Machine-dependent signal context structure for GNU Hurd.  Alpha version.
Copyright (C) 1994 Free Software Foundation, Inc.
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

/* Signal handlers are actually called:
   void handler (int sig, int code, struct sigcontext *scp);  */

/* State of this thread when the signal was taken.  */
struct sigcontext
  {
    /* These first members are machine-independent.  */

    int sc_onstack;		/* Nonzero if running on sigstack.  */
    sigset_t sc_mask;		/* Blocked signals to restore.  */

    /* MiG reply port this thread is using.  */
    unsigned int sc_reply_port;

    /* Port this thread is doing an interruptible RPC on.  */
    unsigned int sc_intr_port;

    /* Error code associated with this signal (interpreted as `error_t').  */
    int sc_error;

    /* All following members are machine-dependent.  The rest of this
       structure is written to be laid out identically to:
       {
         struct alpha_thread_state basic;
         struct alpha_float_state fpu;
       }
       trampoline.c knows this, so it must be changed if this changes.  */

#define sc_alpha_thread_state sc_gs /* Beginning of correspondence.  */
    long int sc_regs[31];	/* General registers $0..$30.  */
    long int sc_pc;		/* Program counter.  */
    long int sc_ps;		/* Processor status.  */

    /* struct alpha_float_state */
#define sc_alpha_float_state sc_fpregs
    long int sc_fpregs[32];	/* Floating point registers $f0..$f31.  */
  };
