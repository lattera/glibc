/* Copyright (C) 2001, 2005, 2009, 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

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

#include <errno.h>
#include <sysdep.h>
#include <setjmp.h>
#include <bits/setjmp.h>
#include <stdlib.h>
#include <unistd.h>

/* Jump to the position specified by ENV, causing the
   setjmp call there to return VAL, or 1 if VAL is 0.  */
void
__longjmp (__jmp_buf env, int val)
{
#ifdef PTR_DEMANGLE
  uintptr_t guard = THREAD_GET_POINTER_GUARD ();
# ifdef CHECK_SP
  CHECK_SP (env, guard);
# endif
#elif defined CHECK_SP
  CHECK_SP (env, 0);
#endif
  register long int r2 __asm ("%r2") = val == 0 ? 1 : val;
#ifdef PTR_DEMANGLE
  register uintptr_t r3 __asm ("%r3") = guard;
  register void *r1 __asm ("%r1") = (void *) env;
#endif
  /* Restore registers and jump back.  */
  asm volatile ("ld   %%f7,104(%1)\n\t"
		"ld   %%f5,96(%1)\n\t"
		"ld   %%f3,88(%1)\n\t"
		"ld   %%f1,80(%1)\n\t"
#ifdef PTR_DEMANGLE
		"lmg  %%r6,%%r13,0(%1)\n\t"
		"lmg  %%r4,%%r5,64(%1)\n\t"
		"xgr  %%r4,%2\n\t"
		"xgr  %%r5,%2\n\t"
		"lgr  %%r15,%%r5\n\t"
		"br   %%r4"
#else
		"lmg  %%r6,%%r15,0(%1)\n\t"
		"br   %%r14"
#endif
		: : "r" (r2),
#ifdef PTR_DEMANGLE
		    "r" (r1), "r" (r3)
#else
		    "a" (env)
#endif
		);

  /* Avoid `volatile function does return' warnings.  */
  for (;;);
}
