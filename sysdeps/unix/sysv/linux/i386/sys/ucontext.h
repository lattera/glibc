/* Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
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
#define NGREG	19

/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

#ifdef __USE_GNU
/* Number of each register is the `gregset_t' array.  */
enum
{
  GS = 0,
# define GS	GS
  FS,
# define FS	FS
  ES,
# define ES	ES
  DS,
# define DS	DS
  EDI,
# define EDI	EDI
  ESI,
# define ESI	ESI
  EBP,
# define EBP	EBP
  ESP,
# define ESP	ESP
  EBX,
# define EBX	EBX
  EDX,
# define EDX	EDX
  ECX,
# define ECX	ECX
  EAX,
# define EAX	EAX
  TRAPNO,
# define TRAPNO	TRAPNO
  ERR,
# define ERR	ERR
  EIP,
# define EIP	EIP
  CS,
# define CS	CS
  EFL,
# define EFL	EFL
  UESP,
# define UESP	UESP
  SS
# define SS	SS
};
#endif

/* Structure to describe FPU registers.  */
typedef struct _fpstate *fpregset_t;

/* Context to describe whole processor state.  */
typedef struct
  {
    gregset_t gregs;
    /* Due to Linux's history we have to use a pointer here.  The SysV/i386
       ABI requires a struct with the values.  */
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
    struct _fpstate __fpregs_mem;
  } ucontext_t;

#endif /* sys/ucontext.h */
