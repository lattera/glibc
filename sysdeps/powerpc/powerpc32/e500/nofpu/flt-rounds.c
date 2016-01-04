/* Return current rounding mode as correct value for FLT_ROUNDS.  e500
   version.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv_libc.h>
#include <stdlib.h>

int
__flt_rounds (void)
{
  switch (fegetenv_register () & SPEFSCR_FRMC)
    {
    case FE_TOWARDZERO:
      return 0;
    case FE_TONEAREST:
      return 1;
    case FE_UPWARD:
      return 2;
    case FE_DOWNWARD:
      return 3;
    default:
      abort ();
    }
}
