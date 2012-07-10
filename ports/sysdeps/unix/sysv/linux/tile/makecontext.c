/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ucontext.h>
#include <arch/abi.h>

void
__makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
  extern void __startcontext (void);
  uint_reg_t *sp, *args;
  va_list ap;
  int i;

  /* Initialize the top of stack. */
  sp = (uint_reg_t *) ((((intptr_t) ucp->uc_stack.ss_sp
                         + ucp->uc_stack.ss_size) & -16L) - 16);

  /* Allow room for memory-passed arguments if necessary. */
  if (argc > 10)
    sp -= 2 + (argc - 10);

  sp[0] = sp[1] = 0;

  /* Set parameters.  */
  va_start (ap, argc);
  args = &ucp->uc_mcontext.gregs[0];
  for (i = 0; i < argc; i++)
    {
      if (i == 10)
        args = &sp[2];
      *args++ = va_arg (ap, long);
    }
  va_end (ap);

  /* Pass (*func) to __startcontext in pc.  */
  ucp->uc_mcontext.pc = (long) func;

  /* Set stack pointer.  */
  ucp->uc_mcontext.sp = (long) sp;

  /* Set the return address to trampoline.  */
  ucp->uc_mcontext.lr = (long) __startcontext;

  /* Pass ucp->uc_link to __startcontext in r30.  */
  ucp->uc_mcontext.gregs[30] = (long) ucp->uc_link;
}
weak_alias (__makecontext, makecontext)
