/* Copyright (C) 1991, 1992, 1994, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif

/* Put these global register declarations first, because we get an error if
   they come after any function definition, including inlines which might
   be in some header.  */

#define REGS \
  REG (bx);\
  REG (si);\
  REG (di);\
  REG (bp);\
  REG (sp)

#define REG(xx) register long int xx asm (#xx)
REGS;
#undef	REG

#include <ansidecl.h>
#include <errno.h>
#include <setjmp.h>
#include <stdlib.h>

/* Jump to the position specified by ENV, causing the
   setjmp call there to return VAL, or 1 if VAL is 0.  */
void
DEFUN(__longjmp, (env, val),
      __jmp_buf env AND int val)
{
  /* We specify explicit registers because, when not optimizing,
     the compiler will generate code that uses the frame pointer
     after it's been munged.  */

  register CONST __typeof (env[0]) *e asm ("cx");
  register int v asm ("ax");

  e = env;
  v = val == 0 ? 1 : val;

#define	REG(xx)	xx = (long int) e->__##xx
  REGS;

  asm volatile ("jmp %*%0" : : "g" (e->__pc), "a" (v));

  /* NOTREACHED */
  abort ();
}
