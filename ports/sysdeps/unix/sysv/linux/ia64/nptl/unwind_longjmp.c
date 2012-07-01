/* Copyright (C) 1991,92,94,95,97,98,2000,02,04 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <setjmp.h>
#include <signal.h>
#include <pthreadP.h>
#include <jmpbuf-unwind.h>

extern void __sigstack_longjmp (__jmp_buf, int)
     __attribute__ ((noreturn));

/* Like __libc_siglongjmp(), but safe for crossing from alternate
   signal stack to normal stack.  Needed by NPTL.  */
void
__libc_unwind_longjmp (sigjmp_buf env, int val)
{
  /* Perform any cleanups needed by the frames being unwound.  */
  __pthread_cleanup_upto (env->__jmpbuf, CURRENT_STACK_FRAME);

  if (env[0].__mask_was_saved)
    /* Restore the saved signal mask.  */
    (void) __sigprocmask (SIG_SETMASK, &env[0].__saved_mask,
			  (sigset_t *) NULL);

  /* Call the machine-dependent function to restore machine state.  */
  __sigstack_longjmp (env[0].__jmpbuf, val ?: 1);
}
hidden_def (__libc_unwind_longjmp)
