/* Compatibility functions for floating point formatting, reentrant versions.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
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

#include <errno.h>
#include <float.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <sys/param.h>
#include <math_ldbl_opt.h>

#ifndef FLOAT_TYPE
# define FLOAT_TYPE double
# define FUNC_PREFIX
# define FLOAT_FMT_FLAG
# define FLOAT_NAME_EXT
# define FLOAT_MIN_10_EXP DBL_MIN_10_EXP
# if DBL_MANT_DIG == 53
#  define NDIGIT_MAX 17
# elif DBL_MANT_DIG == 24
#  define NDIGIT_MAX 9
# elif DBL_MANT_DIG == 56
#  define NDIGIT_MAX 18
# else
/* See IEEE 854 5.6, table 2 for this formula.  Unfortunately we need a
   compile time constant here, so we cannot use it.  */
#  error "NDIGIT_MAX must be precomputed"
#  define NDIGIT_MAX (lrint (ceil (M_LN2 / M_LN10 * DBL_MANT_DIG + 1.0)))
# endif
# if DBL_MIN_10_EXP == -37
#  define FLOAT_MIN_10_NORM	1.0e-37
# elif DBL_MIN_10_EXP == -307
#  define FLOAT_MIN_10_NORM	1.0e-307
# elif DBL_MIN_10_EXP == -4931
#  define FLOAT_MIN_10_NORM	1.0e-4931
# else
/* libc can't depend on libm.  */
#  error "FLOAT_MIN_10_NORM must be precomputed"
#  define FLOAT_MIN_10_NORM	exp10 (DBL_MIN_10_EXP)
# endif
#else
# define LONG_DOUBLE_CVT
#endif

#define APPEND(a, b) APPEND2 (a, b)
#define APPEND2(a, b) a##b
#define __APPEND(a, b) __APPEND2 (a, b)
#define __APPEND2(a, b) __##a##b

#define FLOOR APPEND(floor, FLOAT_NAME_EXT)
#define FABS APPEND(fabs, FLOAT_NAME_EXT)
#define LOG10 APPEND(log10, FLOAT_NAME_EXT)
#define EXP APPEND(exp, FLOAT_NAME_EXT)


int
__APPEND (FUNC_PREFIX, fcvt_r) (FLOAT_TYPE value, int ndigit, int *decpt,
				int *sign, char *buf, size_t len)
{
  ssize_t n;
  ssize_t i;
  int left;

  if (buf == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  left = 0;
  if (isfinite (value))
    {
      *sign = signbit (value) != 0;
      if (*sign)
	value = -value;

      if (ndigit < 0)
	{
	  /* Rounding to the left of the decimal point.  */
	  while (ndigit < 0)
	    {
	      FLOAT_TYPE new_value = value * 0.1;

	      if (new_value < 1.0)
		{
		  ndigit = 0;
		  break;
		}

	      value = new_value;
	      ++left;
	      ++ndigit;
	    }
	}
    }
  else
    /* Value is Inf or NaN.  */
    *sign = 0;

  n = __snprintf (buf, len, "%.*" FLOAT_FMT_FLAG "f", MIN (ndigit, NDIGIT_MAX),
		  value);
  /* Check for a too small buffer.  */
  if (n >= (ssize_t) len)
    return -1;

  i = 0;
  while (i < n && isdigit (buf[i]))
    ++i;
  *decpt = i;

  if (i == 0)
    /* Value is Inf or NaN.  */
    return 0;

  if (i < n)
    {
      do
	++i;
      while (i < n && !isdigit (buf[i]));

      if (*decpt == 1 && buf[0] == '0' && value != 0.0)
	{
	  /* We must not have leading zeroes.  Strip them all out and
	     adjust *DECPT if necessary.  */
	  --*decpt;
	  while (i < n && buf[i] == '0')
	    {
	      --*decpt;
	      ++i;
	    }
	}

      memmove (&buf[MAX (*decpt, 0)], &buf[i], n - i);
      buf[n - (i - MAX (*decpt, 0))] = '\0';
    }

  if (left)
    {
      *decpt += left;
      if ((ssize_t) --len > n)
	{
	  while (left-- > 0 && n < (ssize_t) len)
	    buf[n++] = '0';
	  buf[n] = '\0';
	}
    }

  return 0;
}

int
__APPEND (FUNC_PREFIX, ecvt_r) (FLOAT_TYPE value, int ndigit, int *decpt,
				int *sign, char *buf, size_t len)
{
  int exponent = 0;

  if (isfinite (value) && value != 0.0)
    {
      /* Slow code that doesn't require -lm functions.  */
      FLOAT_TYPE d;
      FLOAT_TYPE f = 1.0;
      if (value < 0.0)
	d = -value;
      else
	d = value;
      /* For denormalized numbers the d < 1.0 case below won't work,
	 as f can overflow to +Inf.  */
      if (d < FLOAT_MIN_10_NORM)
	{
	  value /= FLOAT_MIN_10_NORM;
	  if (value < 0.0)
	    d = -value;
	  else
	    d = value;
	  exponent += FLOAT_MIN_10_EXP;
	}
      if (d < 1.0)
	{
	  do
	    {
	      f *= 10.0;
	      --exponent;
	    }
	  while (d * f < 1.0);

	  value *= f;
	}
      else if (d >= 10.0)
	{
	  do
	    {
	      f *= 10;
	      ++exponent;
	    }
	  while (d >= f * 10.0);

	  value /= f;
	}
    }
  else if (value == 0.0)
    /* SUSv2 leaves it unspecified whether *DECPT is 0 or 1 for 0.0.
       This could be changed to -1 if we want to return 0.  */
    exponent = 0;

  if (ndigit <= 0 && len > 0)
    {
      buf[0] = '\0';
      *decpt = 1;
      *sign = isfinite (value) ? signbit (value) != 0 : 0;
    }
  else
    if (__APPEND (FUNC_PREFIX, fcvt_r) (value, MIN (ndigit, NDIGIT_MAX) - 1,
					decpt, sign, buf, len))
      return -1;

  *decpt += exponent;
  return 0;
}

#if LONG_DOUBLE_COMPAT (libc, GLIBC_2_0)
# ifdef LONG_DOUBLE_CVT
#  define cvt_symbol(symbol) \
  cvt_symbol_1 (libc, __APPEND (FUNC_PREFIX, symbol), \
	      APPEND (FUNC_PREFIX, symbol), GLIBC_2_4)
#  define cvt_symbol_1(lib, local, symbol, version) \
    versioned_symbol (lib, local, symbol, version)
# else
#  define cvt_symbol(symbol) \
  cvt_symbol_1 (libc, __APPEND (FUNC_PREFIX, symbol), \
	      APPEND (q, symbol), GLIBC_2_0); \
  weak_alias (__APPEND (FUNC_PREFIX, symbol), APPEND (FUNC_PREFIX, symbol))
#  define cvt_symbol_1(lib, local, symbol, version) \
  compat_symbol (lib, local, symbol, version)
# endif
#else
# define cvt_symbol(symbol) \
  weak_alias (__APPEND (FUNC_PREFIX, symbol), APPEND (FUNC_PREFIX, symbol))
#endif
cvt_symbol(fcvt_r);
cvt_symbol(ecvt_r);
