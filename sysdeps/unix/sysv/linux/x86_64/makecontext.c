/* Create new context.
   Copyright (C) 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2002.

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

#include <sysdep.h>
#include <stdarg.h>
#include <ucontext.h>

#include "ucontext_i.h"

/* This implementation can handle any ARGC value but only
   normal integer parameters.
   makecontext sets up a stack and the registers for the
   user context. The stack looks like this:
               +-----------------------+
               | next context          |
               +-----------------------+
               | parameter 7-n         |
	       +-----------------------+
	       | trampoline address    |
    %rsp ->    +-----------------------+

   The registers are set up like this:
     %rdi,%rsi,%rdx,%rcx,%r8,%r9: parameter 1 to 6
     %rbx   : address of next context
     %rsp   : stack pointer.
*/

/* XXX: This implementation currently only handles integer arguments.
   To handle long int and pointer arguments the va_arg arguments needs
   to be changed to long and also the stdlib/tst-setcontext.c file needs
   to be changed to pass long arguments to makecontext.  */


void
__makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
  extern void __start_context (void);
  unsigned long *sp, idx_uc_link;
  va_list ap;
  int i;

  /* Generate room on stack for parameter if needed and uc_link.  */
  sp = (long *) ((long) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);
  sp -= (argc > 6 ? argc - 6 : 0) + 1;
  /* Align stack and make space for trampoline address.  */
  sp = (long *) ((((long) sp) & -16L) - 8);

  idx_uc_link = (argc > 6 ? argc - 6 : 0) + 1;

  /* Setup context ucp.  */
  /* Address to jump to.  */
  ucp->uc_mcontext.gregs[REG_RIP] = (long) func;
  /* Setup rbx.*/
  ucp->uc_mcontext.gregs[REG_RBX] = (long) &sp[idx_uc_link];
  ucp->uc_mcontext.gregs[REG_RSP] = (long) sp;

  /* Setup stack.  */
  sp[0] = (long) &__start_context;
  sp[idx_uc_link] = (long) ucp->uc_link;

  va_start (ap, argc);
  /* Handle arguments.  */
  for (i = 0; i < argc; ++i)
    switch (i)
      {
      case 0:
	ucp->uc_mcontext.gregs [REG_RDI] = va_arg (ap, int);
	break;
      case 1:
	ucp->uc_mcontext.gregs [REG_RSI] = va_arg (ap, int);
	break;
      case 2:
	ucp->uc_mcontext.gregs [REG_RDX] = va_arg (ap, int);
	break;
      case 3:
	ucp->uc_mcontext.gregs [REG_RCX] = va_arg (ap, int);
	break;
      case 4:
	ucp->uc_mcontext.gregs [REG_R8] = va_arg (ap, int);
	break;
      case 5:
	ucp->uc_mcontext.gregs [REG_R9] = va_arg (ap, int);
	break;
      default:
	/* Put value on stack.  */
	sp[(i - 5)] = va_arg (ap, int);
	break;
      }
  va_end (ap);

}


weak_alias(__makecontext, makecontext)
