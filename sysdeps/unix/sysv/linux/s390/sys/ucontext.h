/* Copyright (C) 2000-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

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

/* Type for a program status word.  */
typedef struct
{
  unsigned long mask;
  unsigned long addr;
} __attribute__ ((__aligned__(8))) __psw_t;

/* Type for a general-purpose register.  */
typedef unsigned long greg_t;

/* And the whole bunch of them.  We should have used `struct s390_regs',
   but to avoid name space pollution and since the tradition says that
   the register set is an array, we make gregset_t a simple array
   that has the same size as s390_regs.  This is needed for the
   elf_prstatus structure.  */
#if __WORDSIZE == 64
# define NGREG 27
#else
# define NGREG 36
#endif
/* Must match kernels psw_t alignment.  */
typedef greg_t gregset_t[NGREG] __attribute__ ((__aligned__(8)));

typedef union
  {
    double  d;
    float   f;
  } fpreg_t;

/* Register set for the floating-point registers.  */
typedef struct
  {
    unsigned int fpc;
    fpreg_t fprs[16];
  } fpregset_t;

/* Context to describe whole processor state.  */
typedef struct
  {
    __psw_t psw;
    unsigned long gregs[16];
    unsigned int aregs[16];
    fpregset_t fpregs;
  } mcontext_t;

/* Userlevel context.  */
struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
  };


#endif /* sys/ucontext.h */
