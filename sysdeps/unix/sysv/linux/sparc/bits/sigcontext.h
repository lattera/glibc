/* Copyright (C) 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#if !defined _SIGNAL_H && !defined _SYS_UCONTEXT_H
# error "Never use <bits/sigcontext.h> directly; include <signal.h> instead."
#endif

#include <bits/wordsize.h>

#define SUNOS_MAXWIN   31

/* A register window */
struct reg_window {
  unsigned long locals[8];
  unsigned long ins[8];
};

#if __WORDSIZE == 64

/* This is what SunOS doesn't, so we have to write this alone. */
struct sigcontext {
  int sigc_onstack;      /* state to restore */
  int sigc_mask;         /* sigmask to restore */
  unsigned long sigc_sp;   /* stack pointer */
  unsigned long sigc_pc;   /* program counter */
  unsigned long sigc_npc;  /* next program counter */
  unsigned long sigc_psr;  /* for condition codes etc */
  unsigned long sigc_g1;   /* User uses these two registers */
  unsigned long sigc_o0;   /* within the trampoline code. */

  /* Now comes information regarding the users window set
     at the time of the signal. */
  int sigc_oswins;       /* outstanding windows */

  /* stack ptrs for each regwin buf */
  char *sigc_spbuf[SUNOS_MAXWIN];

  /* Windows to restore after signal */
  struct reg_window sigc_wbuf[SUNOS_MAXWIN];
};

struct pt_regs {
        unsigned long u_regs[16]; /* globals and ins */
        unsigned long tstate;
        unsigned long tpc;
        unsigned long tnpc;
        unsigned int y;
        unsigned int fprs;
};

typedef struct {
        struct     pt_regs si_regs;
        long       si_mask;
} __siginfo_t;

typedef struct {
        unsigned   int si_float_regs [64];
        unsigned   long si_fsr;
        unsigned   long si_gsr;
        unsigned   long si_fprs;
} __siginfo_fpu_t;

#else

/* This is what SunOS does, so shall I. */
struct sigcontext {
  int sigc_onstack;      /* state to restore */
  int sigc_mask;         /* sigmask to restore */
  int sigc_sp;           /* stack pointer */
  int sigc_pc;           /* program counter */
  int sigc_npc;          /* next program counter */
  int sigc_psr;          /* for condition codes etc */
  int sigc_g1;           /* User uses these two registers */
  int sigc_o0;           /* within the trampoline code. */

  /* Now comes information regarding the users window set
     at the time of the signal. */
  int sigc_oswins;       /* outstanding windows */

  /* stack ptrs for each regwin buf */
  char *sigc_spbuf[SUNOS_MAXWIN];

  /* Windows to restore after signal */
  struct reg_window sigc_wbuf[SUNOS_MAXWIN];
};

struct pt_regs {
        unsigned long psr;
        unsigned long pc;
        unsigned long npc;
        unsigned long y;
        unsigned long u_regs[16]; /* globals and ins */
};

typedef struct {
  struct pt_regs  si_regs;
  int             si_mask;
} __siginfo_t;

typedef struct {
  unsigned   long si_float_regs [32];
  unsigned   long si_fsr;
  unsigned   long si_fpqdepth;
  struct {
    unsigned long *insn_addr;
    unsigned long insn;
  } si_fpqueue [16];
} __siginfo_fpu_t;

#endif
