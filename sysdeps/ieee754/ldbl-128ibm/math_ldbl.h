#ifndef _MATH_PRIVATE_H_
#error "Never use <math_ldbl.h> directly; include <math_private.h> instead."
#endif

#include <sysdeps/ieee754/ldbl-128/math_ldbl.h>
#include <ieee754.h>
#include <stdint.h>

static inline void
ldbl_extract_mantissa (int64_t *hi64, uint64_t *lo64, int *exp, long double x)
{
  /* We have 105 bits of mantissa plus one implicit digit.  Since
     106 bits are representable we use the first implicit digit for
     the number before the decimal point and the second implicit bit
     as bit 53 of the mantissa.  */
  uint64_t hi, lo;
  int ediff;
  union ibm_extended_long_double u;
  u.ld = x;
  *exp = u.d[0].ieee.exponent - IEEE754_DOUBLE_BIAS;

  lo = ((uint64_t) u.d[1].ieee.mantissa0 << 32) | u.d[1].ieee.mantissa1;
  hi = ((uint64_t) u.d[0].ieee.mantissa0 << 32) | u.d[0].ieee.mantissa1;
  /* If the lower double is not a denomal or zero then set the hidden
     53rd bit.  */
  if (u.d[1].ieee.exponent > 0x001)
    {
      lo |= (1ULL << 52);
      lo = lo << 7; /* pre-shift lo to match ieee854.  */
      /* The lower double is normalized separately from the upper.  We
	 may need to adjust the lower manitissa to reflect this.  */
      ediff = u.d[0].ieee.exponent - u.d[1].ieee.exponent;
      if (ediff > 53)
	lo = lo >> (ediff-53);
      hi |= (1ULL << 52);
    }

  if ((u.d[0].ieee.negative != u.d[1].ieee.negative)
      && ((u.d[1].ieee.exponent != 0) && (lo != 0LL)))
    {
      hi--;
      lo = (1ULL << 60) - lo;
      if (hi < (1ULL << 52))
	{
	  /* we have a borrow from the hidden bit, so shift left 1.  */
	  hi = (hi << 1) | (lo >> 59);
	  lo = 0xfffffffffffffffLL & (lo << 1);
	  *exp = *exp - 1;
	}
    }
  *lo64 = (hi << 60) | lo;
  *hi64 = hi >> 4;
}

static inline long double
ldbl_insert_mantissa (int sign, int exp, int64_t hi64, u_int64_t lo64)
{
  union ibm_extended_long_double u;
  unsigned long hidden2, lzcount;
  unsigned long long hi, lo;

  u.d[0].ieee.negative = sign;
  u.d[1].ieee.negative = sign;
  u.d[0].ieee.exponent = exp + IEEE754_DOUBLE_BIAS;
  u.d[1].ieee.exponent = exp-53 + IEEE754_DOUBLE_BIAS;
  /* Expect 113 bits (112 bits + hidden) right justified in two longs.
     The low order 53 bits (52 + hidden) go into the lower double */
  lo = (lo64 >> 7)& ((1ULL << 53) - 1);
  hidden2 = (lo64 >> 59) &  1ULL;
  /* The high order 53 bits (52 + hidden) go into the upper double */
  hi = (lo64 >> 60) & ((1ULL << 11) - 1);
  hi |= (hi64 << 4);

  if (lo != 0LL)
    {
      /* hidden2 bit of low double controls rounding of the high double.
	 If hidden2 is '1' then round up hi and adjust lo (2nd mantissa)
	 plus change the sign of the low double to compensate.  */
      if (hidden2)
	{
	  hi++;
	  u.d[1].ieee.negative = !sign;
	  lo = (1ULL << 53) - lo;
	}
      /* The hidden bit of the lo mantissa is zero so we need to
	 normalize the it for the low double.  Shift it left until the
	 hidden bit is '1' then adjust the 2nd exponent accordingly.  */

      if (sizeof (lo) == sizeof (long))
	lzcount = __builtin_clzl (lo);
      else if ((lo >> 32) != 0)
	lzcount = __builtin_clzl ((long) (lo >> 32));
      else
	lzcount = __builtin_clzl ((long) lo) + 32;
      lzcount = lzcount - 11;
      if (lzcount > 0)
	{
	  int expnt2 = u.d[1].ieee.exponent - lzcount;
	  if (expnt2 >= 1)
	    {
	      /* Not denormal.  Normalize and set low exponent.  */
	      lo = lo << lzcount;
	      u.d[1].ieee.exponent = expnt2;
	    }
	  else
	    {
	      /* Is denormal.  */
	      lo = lo << (lzcount + expnt2);
	      u.d[1].ieee.exponent = 0;
	    }
	}
    }
  else
    {
      u.d[1].ieee.negative = 0;
      u.d[1].ieee.exponent = 0;
    }

  u.d[1].ieee.mantissa1 = lo & ((1ULL << 32) - 1);
  u.d[1].ieee.mantissa0 = (lo >> 32) & ((1ULL << 20) - 1);
  u.d[0].ieee.mantissa1 = hi & ((1ULL << 32) - 1);
  u.d[0].ieee.mantissa0 = (hi >> 32) & ((1ULL << 20) - 1);
  return u.ld;
}

/* Handy utility functions to pack/unpack/cononicalize and find the nearbyint
   of long double implemented as double double.  */
static inline long double
default_ldbl_pack (double a, double aa)
{
  union ibm_extended_long_double u;
  u.d[0].d = a;
  u.d[1].d = aa;
  return u.ld;
}

static inline void
default_ldbl_unpack (long double l, double *a, double *aa)
{
  union ibm_extended_long_double u;
  u.ld = l;
  *a = u.d[0].d;
  *aa = u.d[1].d;
}

#ifndef ldbl_pack
# define ldbl_pack   default_ldbl_pack
#endif
#ifndef ldbl_unpack
# define ldbl_unpack default_ldbl_unpack
#endif

/* Convert a finite long double to canonical form.
   Does not handle +/-Inf properly.  */
static inline void
ldbl_canonicalize (double *a, double *aa)
{
  double xh, xl;

  xh = *a + *aa;
  xl = (*a - xh) + *aa;
  *a = xh;
  *aa = xl;
}

/* Simple inline nearbyint (double) function .
   Only works in the default rounding mode
   but is useful in long double rounding functions.  */
static inline double
ldbl_nearbyint (double a)
{
  double two52 = 0x10000000000000LL;

  if (__builtin_expect ((__builtin_fabs (a) < two52), 1))
    {
      if (__builtin_expect ((a > 0.0), 1))
	{
	  a += two52;
	  a -= two52;
	}
      else if (__builtin_expect ((a < 0.0), 1))
	{
	  a = two52 - a;
	  a = -(a - two52);
	}
    }
  return a;
}
