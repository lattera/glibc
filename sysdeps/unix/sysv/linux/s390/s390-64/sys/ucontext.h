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

/* Type for a program status word.  */
typedef struct
{
  unsigned long mask;
  unsigned long addr;
} __psw_t __attribute__ ((aligned(8)));

/* Type for a general-purpose register.  */
typedef unsigned long greg_t;

#define NGREG 16

typedef greg_t gregset_t[NGREG];

typedef union
{
  double  d;
  float   f;
} fpreg_t;

/* Register set for the floating-point registers.  */
typedef struct {
  unsigned int fpc;
  fpreg_t fprs[16];
} fpregset_t;

/* Context to describe whole processor state.  */
typedef struct
  {
    __psw_t      psw;
    gregset_t    gregs;
    unsigned int aregs[16];
    fpregset_t   fpregs;
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
