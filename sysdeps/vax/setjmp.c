/* Copyright (C) 1991, 1992, 1994, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Derived from @(#)_setjmp.s	5.7 (Berkeley) 6/27/88,
   Copyright (c) 1980 Regents of the University of California.

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
