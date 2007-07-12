/* Set the FPU control word.
   Copyright (C) 1996, 1997, 1999 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <math.h>
#include <fpu_control.h>

void
__setfpucw (fpu_control_t set)
{
  fpu_control_t cw;

  /* Fetch the current control word.  */
  _FPU_GETCW (cw);

  /* Preserve the reserved bits, and set the rest as the user
     specified (or the default, if the user gave zero).  */
  cw &= _FPU_RESERVED;
  cw |= set & ~_FPU_RESERVED;

  _FPU_SETCW (cw);
}
