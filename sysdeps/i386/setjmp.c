/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
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

/* Put these global register declarations first, because we get an error if
   they come after any function definition, including inlines which might
   be in some header.  */

#define REGS \
  REG (bx);\
  REG (si);\
  REG (di)

#define REG(xx) register long int xx asm (#xx)
REGS;
#undef	REG

#include <errno.h>
#include <setjmp.h>

/* Save the current program position in ENV and return 0.  */
int
__sigsetjmp (jmp_buf env, int savemask)
{
  /* Save the general registers.  */
#define	REG(xx)	env[0].__jmpbuf[0].__##xx = xx
  REGS;
#undef REG

  /* Save the return PC.  */
  env[0].__jmpbuf[0].__pc = ((void **) &env)[-1];

  /* Save caller's FP, not our own.  */
  env[0].__jmpbuf[0].__bp = ((void **) &env)[-2];

  /* Save caller's SP, not our own.  */
  env[0].__jmpbuf[0].__sp = (void *) &env;

  /* Save the signal mask if requested.  */
  return __sigjmp_save (env, savemask);
}
