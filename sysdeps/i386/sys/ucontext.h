/* Copyright (C) 1997, 1999 Free Software Foundation, Inc.
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
  GS = 0,
#define GS	GS
  FS,
#define FS	FS
  ES,
#define ES	ES
  DS,
#define DS	DS
  EDI,
#define EDI	EDI
  ESI,
#define ESI	ESI
  EBP,
#define EBP	EBP
  ESP,
#define ESP	ESP
  EBX,
#define EBX	EBX
  EDX,
#define EDX	EDX
  ECX,
#define ECX	ECX
  EAX,
#define EAX	EAX
  TRAPNO,
#define TRAPNO	TRAPNO
  ERR,
#define ERR	ERR
  EIP,
#define EIP	EIP
  CS,
#define CS	CS
  EFL,
#define EFL	EFL
  UESP,
#define UESP	UESP
  SS
#define SS	SS
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
