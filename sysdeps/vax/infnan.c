/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#else

#include <ansidecl.h>
#include <errno.h>
#include <math.h>

/* Deal with an infinite or NaN result.
   If ERROR is ERANGE, result is +Inf;
   if ERROR is - ERANGE, result is -Inf;
   otherwise result is NaN.
   This will set `errno' to either ERANGE or EDOM,
   and may return an infinity or NaN, or may do something else.  */
double
DEFUN(__infnan, (error), int error)
{
  switch (error)
    {
    case ERANGE:
      errno = ERANGE;
      break;

    case - ERANGE:
      errno = ERANGE;
      break;

    default:
      errno = EDOM;
      break;
    }

  /* Trigger a reserved operand fault.  */
  {
    double result;
    asm volatile("emodd %1, %1, %2, %0, %0" : "=r" (result) :
		 "i" (0), "i" (0x8000));
    return result;
  }
}

#endif
