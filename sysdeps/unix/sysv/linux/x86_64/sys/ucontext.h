/* Copyright (C) 2001 Free Software Foundation, Inc.
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

/* Type for general register.  */
typedef long int greg_t;

/* Number of general registers.  */
#define NGREG	27

/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

#ifdef __USE_GNU
/* Number of each register in the `gregset_t' array.  */
enum
{
  REG_GSFS = 0,
# define REG_GSFS	REG_GSFS
  REG_ESDS,
# define REG_ESDS	REG_ESDS
  REG_R8,
# define REG_R8		REG_R8
  REG_R9,
# define REG_R9		REG_R9
  REG_R10,
# define REG_R10	REG_R10
  REG_R11,
# define REG_R11	REG_R11
  REG_R12,
# define REG_R12	REG_R12
  REG_R13,
# define REG_R13	REG_R13
  REG_R14,
# define REG_R14	REG_R14
  REG_R15,
# define REG_R15	REG_R15
  REG_RDI,
# define REG_RDI	REG_RDI
  REG_RSI,
# define REG_RSI	REG_RSI
  REG_RBP,
# define REG_RBP	REG_RBP
  REG_RSP,
# define REG_RSP	REG_RSP
  REG_RBX,
# define REG_RBX	REG_RBX
  REG_RDX,
# define REG_RDX	REG_RDX
  REG_RCX,
# define REG_RCX	REG_RCX
  REG_RAX,
# define REG_RAX	REG_RAX
  REG_TRAPNO,
# define REG_TRAPNO	REG_TRAPNO
  REG_ERR,
# define REG_ERR	REG_ERR
  REG_RIP,
# define REG_RIP	REG_RIP
  REG_CS,
# define REG_CS		REG_CS
  REG_EFL,
# define REG_EFL	REG_EFL
  REG_URSP,
# define REG_URSP	REG_URSP
  REG_SS
# define REG_SS	REG_SS
};
#endif

/* Definitions taken from the kernel headers.  */
struct _libc_fpreg
{
  unsigned short int significand[4];
  unsigned short int exponent;
};

struct _libc_fpxreg
{
  unsigned short int significand[4];
  unsigned short int exponent;
  unsigned short int padding[3];
};

struct _libc_xmmreg
{
  unsigned long int element[4];
};

struct _libc_fpstate
{
  /* Regular FPU environment.  */
  unsigned long int cw;
  unsigned long int sw;
  unsigned long int tag;
  unsigned long int ipoff;
  unsigned long int cssel;
  unsigned long int dataoff;
  unsigned long int datasel;
  struct _libc_fpreg _st[16];
  unsigned short int status;
  unsigned short int magic;
  /* FXSR FPU environment.  */
  
  unsigned long	int _fxsr_env[6];
  unsigned long	int mxcsr;
  unsigned long	int reserved;
  struct _libc_fpxreg _fxsr_st[8];
  struct _libc_xmmreg _xmm[16];
  unsigned long int padding[32];
};

/* Structure to describe FPU registers.  */
typedef struct _libc_fpstate fpregset_t;

/* Context to describe whole processor state.  */
typedef struct
  {
    gregset_t gregs;
    fpregset_t fpregs;
    unsigned long int oldmask;
    unsigned long int cr2;
  } mcontext_t;

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long int uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;

#endif /* sys/ucontext.h */
