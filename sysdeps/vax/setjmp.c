/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
   Derived from @(#)_setjmp.s	5.7 (Berkeley) 6/27/88,
   Copyright (c) 1980 Regents of the University of California.

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

#include <setjmp.h>


/* Save the current program position in ENV and return 0.  */
int
__sigsetjmp (jmp_buf env, int savemask)
{
  /* Save our caller's FP and PC.  */
  asm ("movl 12(fp), %0" : "=g" (env[0].__jmpbuf[0].__fp));
  asm ("movl 16(fp), %0" : "=g" (env[0].__jmpbuf[0].__pc));

  /* Save the signal mask if requested.  */
  return __sigjmp_save (env, savemask);
}
