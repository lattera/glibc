/* Copyright 1997, 1999, 2000, 2002 Free Software Foundation, Inc.
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

/* AM33/2.0 context switching support.  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* Type for general register.  */
typedef int greg_t;

/* Number of general registers.  */
#define NGREG	28

/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

/* Number of each register is the `gregset_t' array.  */
enum
{
  REG_D0 = 0,
#define REG_D0	REG_D0
  REG_D1,
#define REG_D1	REG_D1
  REG_D2,
#define REG_D2	REG_D2
  REG_D3,
#define REG_D3	REG_D3
  REG_A0,
#define REG_A0	REG_A0
  REG_A1,
#define REG_A1	REG_A1
  REG_A2,
#define REG_A2	REG_A2
  REG_A3,
#define REG_A3	REG_A3
  REG_E0,
#define REG_E0	REG_E0
  REG_E1,
#define REG_E1	REG_E1
  REG_E2,
#define REG_E2	REG_E2
  REG_E3,
#define REG_E3	REG_E3
  REG_E4,
#define REG_E4	REG_E4
  REG_E5,
#define REG_E5	REG_E5
  REG_E6,
#define REG_E6	REG_E6
  REG_E7,
#define REG_E7	REG_E7
  REG_LAR,
#define REG_LAR	REG_LAR
  REG_LIR,
#define REG_LIR	REG_LIR
  REG_MDR,
#define REG_MDR	REG_MDR
  REG_MCVF,
#define REG_MCVF	REG_MCVF
  REG_MCRL,
#define REG_MCRL	REG_MCRL
  REG_MCRH,
#define REG_MCRH	REG_MCRH
  REG_MDRQ,
#define REG_MDRQ	REG_MDRQ
  REG_SP,
#define REG_SP	REG_SP
  REG_EPSW,
#define REG_EPSW	REG_EPSW
  REG_PC,
#define REG_PC	REG_PC
};

typedef int freg_t;

/* Structure to describe FPU registers.  */
typedef struct {
  union {
    double fp_dregs[16];
    float fp_fregs[32];
    freg_t fp_regs[32];
  } regs;
  freg_t fpcr;
} fpregset_t;

/* Context to describe whole processor state.  */
typedef struct
  {
    gregset_t gregs;
    fpregset_t fpregs;
  } mcontext_t;

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    __sigset_t uc_sigmask;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    long int uc_filler[5];
  } ucontext_t;

#endif /* sys/ucontext.h */
