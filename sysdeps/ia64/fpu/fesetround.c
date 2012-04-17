/* Set current rounding direction.
   Copyright (C) 1999, 2000, 2005, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Christian Boissat <Christian.Boissat@cern.ch>, 1999.

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

#include <fenv.h>

int
fesetround (int round)
{
  fenv_t fpsr;

  if (round & ~3)
    return 1;

  /* Get the current state.  */
  __asm__ __volatile__ ("mov.m %0=ar.fpsr" : "=r" (fpsr));

  /* Set the relevant bits.  */
  fpsr = (fpsr & ~(3UL << 10)) | ((fenv_t) round << 10);

  /* Put the new state in effect.  */
  __asm__ __volatile__ ("mov.m ar.fpsr=%0" :: "r" (fpsr) : "memory");

  return 0;
}
libm_hidden_def (fesetround)
