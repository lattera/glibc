/* Copyright (C) 1992, 1995 Free Software Foundation, Inc.
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

/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <ansidecl.h>
#include <math.h>
#include <float.h>
#include <errno.h>
#include "ieee754.h"

double
DEFUN(ldexp, (x, exp),
      double x AND int exp)
{
  union ieee754_double u;
  unsigned int exponent;

  u.d = x;
#define	x u.d

  exponent = u.ieee.exponent;

  /* The order of the tests is carefully chosen to handle
     the usual case first, with no branches taken.  */

  if (exponent != 0)
    {
      /* X is nonzero and not denormalized.  */

      if (exponent <= DBL_MAX_EXP - DBL_MIN_EXP + 1)
  	{
	  /* X is finite.  When EXP < 0, overflow is actually underflow.  */

	  exponent += exp;

	  if (exponent != 0)
	    {
	      if (exponent <= DBL_MAX_EXP - DBL_MIN_EXP + 1)
		{
		  /* In range.  */
		  u.ieee.exponent = exponent;
		  return x;
		}

	      if (exp >= 0)
	      overflow:
		{
		  CONST int negative = u.ieee.negative;
		  u.d = HUGE_VAL;
		  u.ieee.negative = negative;
		  errno = ERANGE;
		  return u.d;
		}

	      if (exponent <= - (unsigned int) (DBL_MANT_DIG + 1))
		{
		  /* Underflow.  */
		  CONST int negative = u.ieee.negative;
		  u.d = 0.0;
		  u.ieee.negative = negative;
		  errno = ERANGE;
		  return u.d;
		}
	    }

	  /* Gradual underflow.  */
	  u.ieee.exponent = 1;
	  u.d *= ldexp (1.0, (int) exponent - 1);
	  if (u.ieee.mantissa0 == 0 && u.ieee.mantissa1 == 0)
	    /* Underflow.  */
	    errno = ERANGE;
	  return u.d;
  	}

      /* X is +-infinity or NaN.  */
      if (u.ieee.mantissa0 == 0 && u.ieee.mantissa1 == 0)
  	{
	  /* X is +-infinity.  */
	  if (exp >= 0)
	    goto overflow;
	  else
	    {
	      /* (infinity * number < 1).  With infinite precision,
		 (infinity / finite) would be infinity, but otherwise it's
		 safest to regard (infinity / 2) as indeterminate.  The
		 infinity might be (2 * finite).  */
	      CONST int negative = u.ieee.negative;
	      u.d = NAN;
	      u.ieee.negative = negative;
	      errno = EDOM;
	      return u.d;
	    }
	}

      /* X is NaN.  */
      errno = EDOM;
      return u.d;
    }

  /* X is zero or denormalized.  */
  if (u.ieee.mantissa0 == 0 && u.ieee.mantissa1 == 0)
    /* X is +-0.0. */
    return x;

  /* X is denormalized.
     Multiplying by 2 ** DBL_MANT_DIG normalizes it;
     we then subtract the DBL_MANT_DIG we added to the exponent.  */
  return ldexp (x * ldexp (1.0, DBL_MANT_DIG), exp - DBL_MANT_DIG);
}

/* Compatibility names for the same function.  */
weak_alias (ldexp, __scalb)
weak_alias (ldexp, scalb)
