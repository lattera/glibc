/* Copyright (C) 2000 Free Software Foundation, Inc.
   Contributed by Denis Joseph Barrow (djbarrow@de.ibm.com).
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

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* We need the signal context definitions even if they are not used
   included in <signal.h>.  */
#include <bits/sigcontext.h>


/* Type for general register.  */
typedef int greg_t;

/* Number of general registers.  */
#define NGREG	16

/* Number of each register is the `gregset_t' array.  */
enum
{
  GPR0 = 0,
#define GPR0 GPR0
  GPR1,
#define GPR1 GPR1
  GPR2,
#define GPR2 GPR2
  GPR3,
#define GPR3 GPR3
  GPR4,
#define GPR4 GPR4
  GPR5,
#define GPR5 GPR5
  GPR6,
#define GPR6 GPR6
  GPR7,
#define GPR7 GPR7
  GPR8,
#define GPR8 GPR8
  GPR9,
#define GPR9 GPR9
  GPRA,
#define GPRA GPRA
  GPRB,
#define GPRB GPRB
  GPRC,
#define GPRC GPRC
  GPRD,
#define GPRD GPRD
  GPRE,
#define GPRE GPRE
  GPRF
#define GPRF GPRF
};

/* Structure to describe FPU registers.  */
typedef long long fpreg_t;

/* Context to describe whole processor state.  */
typedef struct
  {
    int version;
    greg_t gregs[NGREG];
    fpreg_t fpregs[16];
  } mcontext_t;

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_links;
    __sigset_t uc_sigmask;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    long int uc_filler[170];
  } ucontext_t;

#endif /* sys/ucontext.h */
