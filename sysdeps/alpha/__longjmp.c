/* Copyright (C) 1992 Free Software Foundation, Inc.
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

#ifndef __GNUC__
#error This file uses GNU C extensions; you must compile with GCC.
#endif

/*#include <setjmp.h>*/
#include "jmp_buf.h"
#define jmp_buf __jmp_buf

register long int
  r9 asm ("$9"), r10 asm ("$10"), r11 asm ("$11"), r12 asm ("$12"),
  r13 asm ("$13"), r14 asm ("$14");

register long int *fp asm ("$15"), *sp asm ("$30"), *retpc asm ("$26");

#if 1				/* XXX */
register double
  f2 asm ("$f2"), f3 asm ("$f3"), f4 asm ("$f4"), f5 asm ("$f5"),
  f6 asm ("$f6"), f7 asm ("$f7"), f8 asm ("$f8"), f9 asm ("$f9");
#endif

/* Jump to the position specified by ENV, causing the
   setjmp call there to return VAL, or 1 if VAL is 0.

   We declare this function to return an `int';
   in fact, the value being returned is going to the caller of setjmp.  */
volatile void
__longjmp (const jmp_buf env, int val)
{
  /* Restore the integer registers.  */
  r9 = env[0].__9;
  r10 = env[0].__10;
  r11 = env[0].__11;
  r12 = env[0].__12;
  r13 = env[0].__13;
  r14 = env[0].__14;

#if 1				/* XXX */
  /* Restore the floating point registers.  */
  f2 = env[0].__f2;
  f3 = env[0].__f3;
  f4 = env[0].__f4;
  f5 = env[0].__f5;
  f6 = env[0].__f6;
  f7 = env[0].__f7;
  f8 = env[0].__f8;
  f9 = env[0].__f9;
#endif

  /* Set the return PC to that of setjmp's caller.  */
  retpc = env[0].__pc;

  /* Restore the FP and SP of setjmp's caller.  */
  fp = env[0].__fp;
  sp = env[0].__sp;

  /* Return VAL (or 1 if VAL is zero) to setjmp's caller.

     We use an asm here rather than a normal C return statement
     just in case the compiler wanted to do some stack frobnication
     in the function epilogue.  Since we have already restored
     precisely the FP and SP the desired environment needs,
     we must avoid the compiler doing anything with the stack.  */

  while (1)
    {
      /* The loop is just to avoid `volatile function does return' warnings.
	 The instruction will only be executed once.  */

      register long int retval asm ("$0");

      asm volatile
	("cmoveq %1, 1, %0\n\t"	/* $0 = val ?: 1; */
	 "ret $31, (%2), 1"	/* return $0 */
	 : "=r" (retval)
	 /* The "0" constraint should force VAL into $0.  */
	 : "0" (val), "r" (retpc));
    }
}
