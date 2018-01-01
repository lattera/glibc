/* Internal header file for <setjmp.h>.  Linux/x86 version.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

#ifndef	_SETJMPP_H
#define	_SETJMPP_H	1

#include <bits/types/__sigset_t.h>

/* The biggest signal number + 1.  As of kernel 4.14, x86 _NSIG is 64.
   Define it to 513 to leave some rooms for future use.  */
#define _JUMP_BUF_SIGSET_NSIG	513
/* Number of longs to hold all signals.  */
#define _JUMP_BUF_SIGSET_NWORDS \
  ((_JUMP_BUF_SIGSET_NSIG - 1 + 7) / (8 * sizeof (unsigned long int)))

typedef struct
  {
    unsigned long int __val[_JUMP_BUF_SIGSET_NWORDS];
  } __jmp_buf_sigset_t;

typedef union
  {
    __sigset_t __saved_mask_compat;
    struct
      {
	__jmp_buf_sigset_t __saved_mask;
	/* Used for shadow stack pointer.  */
	unsigned long int __shadow_stack_pointer;
      } __saved;
  } __jmpbuf_arch_t;

#undef __sigset_t
#define __sigset_t __jmpbuf_arch_t
#include <setjmp.h>
#undef __saved_mask
#define __saved_mask __saved_mask.__saved.__saved_mask

#include <signal.h>

#define _SIGPROCMASK_NSIG_WORDS (_NSIG / (8 * sizeof (unsigned long int)))

typedef struct
  {
    unsigned long int __val[_SIGPROCMASK_NSIG_WORDS];
  } __sigprocmask_sigset_t;

extern jmp_buf ___buf;
extern  __typeof (___buf[0].__saved_mask) ___saved_mask;
_Static_assert (sizeof (___saved_mask) >= sizeof (__sigprocmask_sigset_t),
		"size of ___saved_mask < size of __sigprocmask_sigset_t");

#endif /* setjmpP.h  */
