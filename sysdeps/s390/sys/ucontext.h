/* Copyright (C) 2000 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

/* System V/s390 ABI compliant context switching support.  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* Type for general register.  */
typedef int greg_t;

/* Number of general registers.  */
#define NGREG	16

/* Number of each register is the `greg_t gregs[NREG]' array.  */
enum
{
  R_GPR0 = 0,
#define R_GPR0	R_GPR0
  R_GPR1 = 1,
#define R_GPR1	R_GPR1
  R_GPR2 = 2,
#define R_GPR2	R_GPR2
  R_GPR3 = 3,
#define R_GPR3	R_GPR3
  R_GPR4 = 4,
#define R_GPR4	R_GPR4
  R_GPR5 = 5,
#define R_GPR5	R_GPR5
  R_GPR6 = 6,
#define R_GPR6	R_GPR6
  R_GPR7 = 7,
#define R_GPR7	R_GPR7
  R_GPR8 = 8,
#define R_GPR8	R_GPR8
  R_GPR9 = 9,
#define R_GPR9	R_GPR9
  R_GPRA = 10,
#define R_GPRA	R_GPRA
  R_GPRB = 11,
#define R_GPRB	R_GPRB
  R_GPRC = 12,
#define R_GPRC	R_GPRC
  R_GPRD = 13,
#define R_GPRD	R_GPRD
  R_GPRE = 14,
#define R_GPRE	R_GPRE
  R_GPRF = 15
#define R_GPRF	R_GPRF
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

#define MCONTEXT_VERSION 1

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
