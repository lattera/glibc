/* Copyright (C) 1998-2017 Free Software Foundation, Inc.

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

/* System V/AArch64 ABI compliant context switching support.  */

#ifndef _SYS_UCONTEXT_H
#define _SYS_UCONTEXT_H	1

#include <features.h>

#include <bits/types/sigset_t.h>
#include <bits/sigcontext.h>
#include <bits/types/stack_t.h>

#ifdef __USE_MISC
# include <sys/procfs.h>


typedef elf_greg_t greg_t;

/* Container for all general registers.  */
typedef elf_gregset_t gregset_t;

/* Structure to describe FPU registers.  */
typedef elf_fpregset_t	fpregset_t;
#endif

/* Context to describe whole processor state.  This only describes
   the core registers; coprocessor registers get saved elsewhere
   (e.g. in uc_regspace, or somewhere unspecified on the stack
   during non-RT signal handlers).  */
typedef struct sigcontext mcontext_t;

/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    sigset_t uc_sigmask;
    mcontext_t uc_mcontext;
  } ucontext_t;

#endif /* sys/ucontext.h */
