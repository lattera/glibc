/* Create new context.
   Copyright (C) 2008, 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Helge Deller <deller@gmx.de>, 2008.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <libintl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysdep.h>
#include <ucontext.h>

/* POSIX only supports integer arguments.  */
#define STACK_ALIGN 64
#define FRAME_SIZE 8

void
__makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
  unsigned long *sp;
  va_list ap;
  int i;

  /* Get stack pointer (64-byte aligned).  */
  sp = (unsigned long *)((((unsigned long) ucp->uc_stack.ss_sp) 
			 + FRAME_SIZE + argc + STACK_ALIGN) 
		        & ~(STACK_ALIGN - 1));

  /* Store address to jump to.  */
  ucp->uc_mcontext.sc_gr[2] = (unsigned long) func;

  va_start (ap, argc);
  /* Handle arguments.  */
  for (i = 0; i < argc; ++i)
    {
      if (i < 4)
	{
	  ucp->uc_mcontext.sc_gr[26-i] = va_arg (ap, int);
	  continue;
	}

      if ((i < 8) && (sizeof(unsigned long) == 8))
	{
	  /* 64bit: r19-r22 are arg7-arg4.  */
	  ucp->uc_mcontext.sc_gr[22+4-i] = va_arg (ap, int);
	  continue;
	} 

      /* All other arguments go on the stack.  */
      sp[-1 * (FRAME_SIZE + 1 + i)] = va_arg (ap, int);
    }
  va_end (ap); 

  /* Adjust the stack pointer to last used argument.  */
  ucp->uc_mcontext.sc_gr[30] = (unsigned long) sp;
}


weak_alias(__makecontext, makecontext)
