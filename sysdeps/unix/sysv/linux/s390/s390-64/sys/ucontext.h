/* Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Denis Joseph Barrow (djbarrow@de.ibm.com).
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
/* Forward definition to avoid parse errors */
struct ucontext;
typedef struct ucontext ucontext_t;
#include <features.h>
#include <signal.h>

/* We need the signal context definitions even if they are not used
   included in <signal.h>.  */
#include <bits/sigcontext.h>


/* Type for a general-purpose register.  */
typedef unsigned long greg_t;

/* And the whole bunch of them.  We should have used `struct s390_regs',
   but to avoid name space pollution and since the tradition says that
   the register set is an array, we make gregset_t a simple array
   that has the same size as s390_regs. */
#define NGREG 27
#define NUM_FPRS 16
/* Must match kernels psw_t alignment */
typedef greg_t gregset_t[NGREG] __attribute__ ((aligned(8)));

typedef union
{
  double  d;
  float   f;
} fpreg_t;

/* Register set for the floating-point registers.  */
typedef struct {
  unsigned int fpc;
  fpreg_t fprs[NUM_FPRS];
} fpregset_t;

/* Context to describe whole processor state.  */
typedef struct
  {
    int version;
    gregset_t    gregs;
    fpregset_t   fpregs;
  } mcontext_t;

/* Userlevel context.  */
struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    __sigset_t uc_sigmask;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    long int uc_filler[170];
  };


#endif /* sys/ucontext.h */
