/* Copyright (C) 1998, 1999, 2002 Free Software Foundation, Inc.
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

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* We need the signal context definitions even if they are not used
   included in <signal.h>.  */
#include <bits/sigcontext.h>

#if __WORDSIZE == 32

/* Number of general registers.  */
#define NGREG	48

/* Container for all general registers.  */
typedef unsigned long gregset_t[NGREG];

/* Container for floating-point registers and status */
typedef struct _libc_fpstate
{
	double fpregs[32];
	double fpscr;
	unsigned int _pad[2];
} fpregset_t;

/* Container for Altivec/VMX registers and status.
   Needs to be aligned on a 16-byte boundary. */
typedef struct _libc_vrstate
{
	unsigned int vrregs[32][4];
	unsigned int vscr;
	unsigned int vrsave;
	unsigned int _pad[2];
} vrregset_t;

/* Context to describe whole processor state.  */
typedef struct
{
	gregset_t gregs;
	fpregset_t fpregs;
	vrregset_t vrregs __attribute__((__aligned__(16)));
} mcontext_t;

#else

/* For 64-bit, a machine context is exactly a sigcontext.  */
typedef struct sigcontext mcontext_t;

#endif

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
#if __WORDSIZE == 32
    /* These fields are for backwards compatibility. */
    int uc_pad[7];
    mcontext_t *uc_regs;
    unsigned int uc_oldsigmask[2];
    int uc_pad2;
#endif
    sigset_t    uc_sigmask;
    mcontext_t  uc_mcontext;  /* last for extensibility */
  } ucontext_t;

#endif /* sys/ucontext.h */
