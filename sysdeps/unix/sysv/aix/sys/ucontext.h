/* Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
#include <bits/types.h>
#include <bits/sigset.h>


/* Alternate, preferred interface.  */
typedef struct sigaltstack
  {
    void *ss_sp;
    size_t ss_size;
    int ss_flags;
    int __pad[4];
  } stack_t;


/* Forward declaration of AIX type.  */
typedef struct label_t label_t;



typedef unsigned int kvmhandle_t;
typedef struct
  {
    unsigned long int __alloc;
    kvmhandle_t __srval[16];
  } adspace_t;



#define _NGPRS 32
#define _NFPRS 32

struct __mstsafe
{
  struct __mstsave *__prev;		/* Previous save area. */
  label_t *__kjmpbuf;			/* Pointer to saved context.  */
  char *__stackfix;			/* Stack fix pointer.  */
  char __intpri;			/* Interrupt priority.  */
  char __backt;				/* Back-track flag.  */
  char __rsvd[2];			/* Reserved.  */
  __pid_t __curid;			/* Copy of curid.  */

  int __excp_type;			/* Exception type for debugger.  */
  unsigned long int __iar;		/* Instruction address register.  */
  unsigned long int __msr;		/* Machine state register.  */
  unsigned long int __cr;		/* Condition register.  */
  unsigned long int __lr;		/* Link register.  */
  unsigned long int __ctr;		/* Count register.  */
  unsigned long int __xer;		/* Fixed point exception.  */
  unsigned long int __mq;		/* Multiply/quotient register.  */
  unsigned long int __tid;		/* TID register.  */
  unsigned long int __fpscr;		/* Floating point status reg.  */
  char __fpeu;				/* Floating point ever used.  */
  char __fpinfo;			/* Floating point status flags.  */
  char __pad[2];			/* Pad to dword boundary.  */
                                        /* 1 implies state is in mstext */
  unsigned long int __except[5];	/* exception structure.  */
  char __pad1[4];			/* Old bus field.  */
  unsigned long int __o_iar;		/* Old iar (for longjmp excpt).  */
  unsigned long int __o_toc;		/* Old toc (for longjmp excpt).  */
  unsigned long int __o_arg1;		/* Old arg1 (for longjmp excpt).  */
  unsigned long int __excbranch;	/* If not NULL, address to branch
					   to on exception.  Used by
					   assembler routines for low
					   cost exception handling.  */
  unsigned long int __fpscrx;		/* Software extension to fpscr.  */
  unsigned long int __o_vaddr;		/* Saved vaddr for vmexception.  */
  unsigned long int __cachealign[7];	/* Reserved.  */
  adspace_t __as;			/* Segment registers.  */
  unsigned long int __gpr[_NGPRS];	/* General purpose registers.  */
  double __fpr[_NFPRS];			/* Floating point registers.  */
    };

typedef struct mcontext_t
  {
    struct __mstsafe __jmp_context;
  } mcontext_t;


typedef struct ucontext_t
  {
    int __sc_onstack;		/* Sigstack state to restore.  */
    __sigset_t uc_sigmask;	/* The set of signals that are blocked when
                                   this context is active.  */
    int __sc_uerror;		/* u_error to restore.  */
    mcontext_t uc_mcontext;	/* Machine-specific image of saved context.  */
    struct ucontext_t *uc_link;	/* context resumed after this one returns */
    stack_t uc_stack;		/* stack used by context */
    int __pad[4];
  } ucontext_t;

#endif /* sys/ucontext.h */
