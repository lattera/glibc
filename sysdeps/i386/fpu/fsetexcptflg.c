/* Set floating-point environment exception handling.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <fenv.h>
#include <math.h>

void
fesetexceptflag (const fexcept_t *flagp, int excepts)
{
  fenv_t temp;

  /* Get the current environment.  We have to do this since we cannot
     separately set the status word.  */
  __asm__ ("fnstenv %0" : "=m" (*&temp));

  temp.status_word &= ~(excepts & FE_ALL_EXCEPT);
  temp.status_word |= *flagp & excepts & FE_ALL_EXCEPT;

  /* Store the new status word (along with the rest of the environment.
     Possibly new exceptions are set but they won't get executed unless
     the next floating-point instruction.  */
  __asm__ ("fldenv %0" : : "m" (*&temp));
}
