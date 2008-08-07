/* Create new context.
   Copyright (C) 2008 Free Software Foundation, Inc.
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

/* XXX: This implementation only handles integer arguments.  */

void
__makecontext (ucontext_t *ucp, void (*func) (void), int argc, ...)
{
  unsigned int *sp;
  va_list ap;
  int i;

  if (argc > 8)
    {
      fprintf (stderr, _("\
makecontext: does not know how to handle more than 8 arguments\n"));
      exit (-1);
    }

  /* Get stack pointer.  */
  sp = (unsigned int *) ucp->uc_stack.ss_sp;

  /* Store address to jump to.  */
  ucp->uc_mcontext.sc_gr[2] = (unsigned long) func;

  va_start (ap, argc);
  /* Handle arguments.  */
  for (i = 0; i < argc; ++i)
    switch (i)
      {
      case 0:
      case 1:
      case 2:
      case 3:
      	ucp->uc_mcontext.sc_gr[26-i] = va_arg (ap, int);
	break;
      case 4:
      case 5:
      case 6:
      case 7:
	if (sizeof(unsigned long) == 4) {
		/* 32bit: put arg7-arg4 on stack.  */
		sp[7-i] = va_arg (ap, int);
	} else {
		/* 64bit: r19-r22 are arg7-arg4.  */
		ucp->uc_mcontext.sc_gr[22+4-i] = va_arg (ap, int);
	}
	break;
      }
  va_end (ap);

}


weak_alias(__makecontext, makecontext)
