/* Set floating-point environment exception handling.
   Copyright (C) 1998, 1999, 2000, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>
   based on corresponding file in the MIPS port.

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
#include <fpu_control.h>
#include <shlib-compat.h>

int
__fesetexceptflag (const fexcept_t *flagp, int excepts)
{
  fpu_control_t cw, temp;

  /* Get the current exceptions.  */
  _FPU_GETCW (cw);

  /* Make sure the flags we want restored are legal.  */
  excepts &= FE_ALL_EXCEPT;
  temp = *flagp & excepts;

  /* If EF bits are clear and the user requests them to be set,
     we have to fail, because there's no way to do it.  */
  if (~(cw & excepts) & temp)
    return -1;

  /* We clear EF bits by storing a 1 in them, so flip the
     FE_ALL_EXCEPT bits.  */
  temp = (~temp & FE_ALL_EXCEPT);

  /* Now clear the bits called for, and copy them in from flagp. Note that
     we ignore all non-flag bits from *flagp, so they don't matter.  */
  cw = (cw & ~FE_ALL_EXCEPT) | temp;

  _FPU_SETFCW (cw);

  /* Success.  */
  return 0;
}

versioned_symbol (libm, __fesetexceptflag, fesetexceptflag, GLIBC_2_2);
