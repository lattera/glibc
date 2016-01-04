/* Raise given exceptions, given the SPEFSCR bits for those exceptions.
   Copyright (C) 1997-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv_libc.h>

int
__FERAISEEXCEPT_INTERNAL (int excepts)
{
  unsigned long f;

  f = fegetenv_register ();
  f |= (excepts & SPEFSCR_ALL_EXCEPT);
  fesetenv_register (f);

  /* Force the operations that cause the exceptions.  */
  if ((SPEFSCR_FINVS & excepts) != 0)
    /* 0 / 0 */
    asm volatile ("efsdiv %0,%0,%1" : : "r" (0), "r" (0));

  if ((SPEFSCR_FDBZS & excepts) != 0)
    /* 1.0 / 0.0 */
    asm volatile ("efsdiv %0,%0,%1" : : "r" (1.0F), "r" (0));

  if ((SPEFSCR_FOVFS & excepts) != 0)
    /* Largest normalized number plus itself.  */
    asm volatile ("efsadd %0,%0,%1" : : "r" (0x7f7fffff), "r" (0x7f7fffff));

  if ((SPEFSCR_FUNFS & excepts) != 0)
    /* Smallest normalized number times itself.  */
    asm volatile ("efsmul %0,%0,%1" : : "r" (0x800000), "r" (0x800000));

  if ((SPEFSCR_FINXS & excepts) != 0)
    /* Smallest normalized minus 1.0 raises the inexact flag.  */
    asm volatile ("efssub %0,%0,%1" : : "r" (0x00800000), "r" (1.0F));

  /* Success.  */
  return 0;
}
