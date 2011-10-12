/* Double-precision floating point square root.
   Copyright (C) 1997, 2002-2004, 2008, 2011 Free Software Foundation, Inc.
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
#include <math_private.h>
#include <fenv_libc.h>
#include <inttypes.h>

#include <sysdep.h>
#include <ldsodefs.h>

static const double almost_half = 0.5000000000000001;	/* 0.5 + 2^-53 */
static const ieee_float_shape_type a_nan = {.word = 0x7fc00000 };
static const ieee_float_shape_type a_inf = {.word = 0x7f800000 };
static const float two108 = 3.245185536584267269e+32;
static const float twom54 = 5.551115123125782702e-17;
extern const float __t_sqrt[1024];

/* The method is based on a description in
   Computation of elementary functions on the IBM RISC System/6000 processor,
   P. W. Markstein, IBM J. Res. Develop, 34(1) 1990.
   Basically, it consists of two interleaved Newton-Raphson approximations,
   one to find the actual square root, and one to find its reciprocal
   without the expense of a division operation.   The tricky bit here
   is the use of the POWER/PowerPC multiply-add operation to get the
   required accuracy with high speed.

   The argument reduction works by a combination of table lookup to
   obtain the initial guesses, and some careful modification of the
   generated guesses (which mostly runs on the integer unit, while the
   Newton-Raphson is running on the FPU).  */

double
__slow_ieee754_sqrt (double x)
{
  const float inf = a_inf.value;

  if (x > 0)
    {
      /* schedule the EXTRACT_WORDS to get separation between the store
	 and the load.  */
      ieee_double_shape_type ew_u;
      ieee_double_shape_type iw_u;
      ew_u.value = (x);
      if (x != inf)
	{
	  /* Variables named starting with 's' exist in the
	     argument-reduced space, so that 2 > sx >= 0.5,
	     1.41... > sg >= 0.70.., 0.70.. >= sy > 0.35... .
	     Variables named ending with 'i' are integer versions of
	     floating-point values.  */
	  double sx;	/* The value of which we're trying to find the
			   square root.  */
	  double sg, g;	/* Guess of the square root of x.  */
	  double sd, d;	/* Difference between the square of the guess and x.  */
	  double sy;	/* Estimate of 1/2g (overestimated by 1ulp).  */
	  double sy2;	/* 2*sy */
	  double e;	/* Difference between y*g and 1/2 (se = e * fsy).  */
	  double shx;	/* == sx * fsg */
	  double fsg;	/* sg*fsg == g.  */
	  fenv_t fe;	/* Saved floating-point environment (stores rounding
			   mode and whether the inexact exception is
			   enabled).  */
	  uint32_t xi0, xi1, sxi, fsgi;
	  const float *t_sqrt;

	  fe = fegetenv_register ();
	  /* complete the EXTRACT_WORDS (xi0,xi1,x) operation.  */
	  xi0 = ew_u.parts.msw;
	  xi1 = ew_u.parts.lsw;
	  relax_fenv_state ();
	  sxi = (xi0 & 0x3fffffff) | 0x3fe00000;
	  /* schedule the INSERT_WORDS (sx, sxi, xi1) to get separation
	     between the store and the load.  */
	  iw_u.parts.msw = sxi;
	  iw_u.parts.lsw = xi1;
	  t_sqrt = __t_sqrt + (xi0 >> (52 - 32 - 8 - 1) & 0x3fe);
	  sg = t_sqrt[0];
	  sy = t_sqrt[1];
	  /* complete the INSERT_WORDS (sx, sxi, xi1) operation.  */
	  sx = iw_u.value;

	  /* Here we have three Newton-Raphson iterations each of a
	     division and a square root and the remainder of the
	     argument reduction, all interleaved.   */
	  sd = -(sg * sg - sx);
	  fsgi = (xi0 + 0x40000000) >> 1 & 0x7ff00000;
	  sy2 = sy + sy;
	  sg = sy * sd + sg;	/* 16-bit approximation to sqrt(sx). */

	  /* schedule the INSERT_WORDS (fsg, fsgi, 0) to get separation
	     between the store and the load.  */
	  INSERT_WORDS (fsg, fsgi, 0);
	  iw_u.parts.msw = fsgi;
	  iw_u.parts.lsw = (0);
	  e = -(sy * sg - almost_half);
	  sd = -(sg * sg - sx);
	  if ((xi0 & 0x7ff00000) == 0)
	    goto denorm;
	  sy = sy + e * sy2;
	  sg = sg + sy * sd;	/* 32-bit approximation to sqrt(sx).  */
	  sy2 = sy + sy;
	  /* complete the INSERT_WORDS (fsg, fsgi, 0) operation.  */
	  fsg = iw_u.value;
	  e = -(sy * sg - almost_half);
	  sd = -(sg * sg - sx);
	  sy = sy + e * sy2;
	  shx = sx * fsg;
	  sg = sg + sy * sd;	/* 64-bit approximation to sqrt(sx),
				   but perhaps rounded incorrectly.  */
	  sy2 = sy + sy;
	  g = sg * fsg;
	  e = -(sy * sg - almost_half);
	  d = -(g * sg - shx);
	  sy = sy + e * sy2;
	  fesetenv_register (fe);
	  return g + sy * d;
	denorm:
	  /* For denormalised numbers, we normalise, calculate the
	     square root, and return an adjusted result.  */
	  fesetenv_register (fe);
	  return __slow_ieee754_sqrt (x * two108) * twom54;
	}
    }
  else if (x < 0)
    {
      /* For some reason, some PowerPC32 processors don't implement
	 FE_INVALID_SQRT.  */
#ifdef FE_INVALID_SQRT
      feraiseexcept (FE_INVALID_SQRT);

      fenv_union_t u = { .fenv = fegetenv_register () };
      if ((u.l[1] & FE_INVALID) == 0)
#endif
	feraiseexcept (FE_INVALID);
      x = a_nan.value;
    }
  return f_wash (x);
}

double
__ieee754_sqrt (double x)
{
  double z;

  /* If the CPU is 64-bit we can use the optional FP instructions.  */
  if (__CPU_HAS_FSQRT)
    {
      /* Volatile is required to prevent the compiler from moving the
	 fsqrt instruction above the branch.  */
      __asm __volatile ("	fsqrt	%0,%1\n"
				:"=f" (z):"f" (x));
    }
  else
    z = __slow_ieee754_sqrt (x);

  return z;
}
strong_alias (__ieee754_sqrt, __sqrt_finite)
