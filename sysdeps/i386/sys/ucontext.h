/* Copyright (C) 1997,99,2000 Free Software Foundation, Inc.
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

/* System V/i386 ABI compliant context switching support.  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>
#include <signal.h>

/* Type for general register.  */
typedef int greg_t;

/* Number of general registers.  */
#define NGREG	19

/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

/* Number of each register is the `gregset_t' array.  */
enum
{
  REG_GS = 0,
#define REG_GS	REG_GS
  REG_FS,
#define REG_FS	REG_FS
  REG_ES,
#define REG_ES	REG_ES
  REG_DS,
#define REG_DS	REG_DS
  REG_EDI,
#define REG_EDI	REG_EDI
  REG_ESI,
#define REG_ESI	REG_ESI
  REG_EBP,
#define REG_EBP	REG_EBP
  REG_ESP,
#define REG_ESP	REG_ESP
  REG_EBX,
#define REG_EBX	REG_EBX
  REG_EDX,
#define REG_EDX	REG_EDX
  REG_ECX,
#define REG_ECX	REG_ECX
  REG_EAX,
#define REG_EAX	REG_EAX
  REG_TRAPNO,
#define REG_TRAPNO	REG_TRAPNO
  REG_ERR,
#define REG_ERR	REG_ERR
  REG_EIP,
#define REG_EIP	REG_EIP
  REG_CS,
#define REG_CS	REG_CS
  REG_EFL,
#define REG_EFL	REG_EFL
  REG_UESP,
#define REG_UESP	REG_UESP
  REG_SS
#define REG_SS	REG_SS
};

/* Structure to describe FPU registers.  */
typedef struct fpregset
  {
    union
      {
	struct fpchip_state
	  {
	    int state[27];
	    int status;
	  } fpchip_state;

	struct fp_emul_space
	  {
	    char fp_emul[246];
	    char fp_epad[2];
	  } fp_emul_space;

	int f_fpregs[62];
      } fp_reg_set;

    long int f_wregs[33];
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
