/* Copyright (C) 1998, 2000 Free Software Foundation, Inc.
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

#include <bits/sigcontext.h>

typedef struct
{
  /* Place-holder for interrupt collection state.  */
  long int ics_placeholder[15];
  unsigned long	int ar_fpsr;		/* Floating point status.  */
  unsigned long	int gp;			/* Global data pointer (gr1).  */
  /* scratch registers: */
  unsigned long	int gr8,   gr9, gr10, gr11,       gr13, gr14, gr15;
  unsigned long	int gr16, gr17, gr18, gr19, gr20, gr21, gr22, gr23;
  unsigned long	int gr24, gr25, gr26, gr27, gr28, gr29, gr30, gr31;
  unsigned long	int ar_unat;
  unsigned long	int ar_ec;
  unsigned long	int ar_ccv;
  /* RSE state: */
  unsigned long	int ar_bsp_base;	/* Location of RSE spill area.  */
  unsigned long	int ar_pfs;
  unsigned long	int ar_rsc;
  unsigned long int ar_bspstore;
  unsigned long	int ar_rnat;
  /* Misc. state: */
  unsigned long	int dirty;		/* BSP - BSPSTORE */
  unsigned long int cr_tpr;		/* Hw interrupt mask register.  */
  unsigned long	int tpdp;		/* thread private data pointer.  */
  unsigned long	int br_6, br_7;
  /* Argument regs (gr32-gr39):  */
  unsigned long	int arg0, arg1, arg2, arg3;
  unsigned long	int arg4, arg5, arg6, arg7;
  unsigned long	int ss_flags;		/* Save state flags.  */
  unsigned long int br_1, br_2, br_3, br_4, br_5; /* Branch registers.  */
  unsigned long int p_regs;		/* Predicates.  */
} mcountext_;


typedef struct ucontext
{
  mcontext_t uc_mcontext;		/* saved machine state */
  int uc_spares[8];			/* room to grow... */
  unsigned int uc_created_by_getcontext: 1;
  unsigned int uc_reserved_flags: 31;
  struct ucontext *uc_link;
  __sigset_t uc_sigmask;
  stack_t uc_stack;
} ucontext_t;

#endif /* sys/ucontext.h */
