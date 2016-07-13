/* sigstack, sigaltstack definitions.
   Copyright (C) 1998-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _BITS_SIGSTACK_H
#define _BITS_SIGSTACK_H 1

#if !defined _SIGNAL_H && !defined _SYS_UCONTEXT_H
# error "Never include this file directly.  Use <signal.h> instead"
#endif

#define __need_size_t
#include <stddef.h>

/* Structure describing a signal stack (obsolete).  */
struct sigstack
  {
    void *ss_sp;		/* Signal stack pointer.  */
    int ss_onstack;		/* Nonzero if executing on this stack.  */
  };

/* Alternate, preferred interface.  */
typedef struct sigaltstack
  {
    void *ss_sp;
    size_t ss_size;
    int ss_flags;
  } stack_t;

/* Possible values for `ss_flags'.  */
enum
{
  SS_ONSTACK = 1,
#define SS_ONSTACK	SS_ONSTACK
  SS_DISABLE
#define SS_DISABLE	SS_DISABLE
};

/* Minimum stack size for a signal handler.  */
#define MINSIGSTKSZ	2048

/* System default stack size.  */
#define SIGSTKSZ	8192

#endif /* bits/sigstack.h */
