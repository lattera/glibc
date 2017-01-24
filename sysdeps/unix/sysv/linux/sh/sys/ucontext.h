/* Copyright (C) 1999-2017 Free Software Foundation, Inc.
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

/* Where is System V/SH ABI?  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* We need the signal context definitions even if they are not exposed
   by <signal.h>.  */
#include <bits/sigcontext.h>
#include <bits/sigstack.h>


typedef int greg_t;

/* Number of general registers.  */
#define NGREG	16

/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

/* Number of each register is the `gregset_t' array.  */
enum
{
  REG_R0 = 0,
#define REG_R0	REG_R0
  REG_R1 = 1,
#define REG_R1	REG_R1
  REG_R2 = 2,
#define REG_R2	REG_R2
  REG_R3 = 3,
#define REG_R3	REG_R3
  REG_R4 = 4,
#define REG_R4	REG_R4
  REG_R5 = 5,
#define REG_R5	REG_R5
  REG_R6 = 6,
#define REG_R6	REG_R6
  REG_R7 = 7,
#define REG_R7	REG_R7
  REG_R8 = 8,
#define REG_R8	REG_R8
  REG_R9 = 9,
#define REG_R9	REG_R9
  REG_R10 = 10,
#define REG_R10	REG_R10
  REG_R11 = 11,
#define REG_R11	REG_R11
  REG_R12 = 12,
#define REG_R12	REG_R12
  REG_R13 = 13,
#define REG_R13	REG_R13
  REG_R14 = 14,
#define REG_R14	REG_R14
  REG_R15 = 15,
#define REG_R15	REG_R15
};

typedef int freg_t;

/* Number of FPU registers.  */
#define NFPREG	16

/* Structure to describe FPU registers.  */
typedef freg_t fpregset_t[NFPREG];

/* Context to describe whole processor state.  */
typedef struct
  {
    unsigned int oldmask;
    gregset_t gregs;
    unsigned int pc;
    unsigned int pr;
    unsigned int sr;
    unsigned int gbr;
    unsigned int mach;
    unsigned int macl;
    fpregset_t fpregs;
    fpregset_t xfpregs;
    unsigned int fpscr;
    unsigned int fpul;
    unsigned int ownedfp;
  } mcontext_t;

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
  } ucontext_t;

#endif /* sys/ucontext.h */
