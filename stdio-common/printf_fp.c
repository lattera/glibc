/* Floating point output for `printf'.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* The gmp headers need some configuration frobs.  */
#define HAVE_ALLOCA 1

#ifdef USE_IN_LIBIO
#  include <libioP.h>
#else
#  include <stdio.h>
#endif
#include <alloca.h>
#include <ctype.h>
#include <float.h>
#include <gmp-mparam.h>
#include <stdlib/gmp.h>
#include <stdlib/gmp-impl.h>
#include <stdlib/longlong.h>
#include <stdlib/fpioconst.h>
#include <locale/localeinfo.h>
#include <limits.h>
#include <math.h>
#include <printf.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef NDEBUG
# define NDEBUG			/* Undefine this for debugging assertions.  */
#endif
#include <assert.h>

/* This defines make it possible to use the same code for GNU C library and
   the GNU I/O library.	 */
#ifdef USE_IN_LIBIO
#  define PUT(f, s, n) _IO_sputn (f, s, n)
#  define PAD(f, c, n) _IO_padn (f, c, n)
/* We use this file GNU C library and GNU I/O library.	So make
   names equal.	 */
#  undef putc
#  define putc(c, f) _IO_putc_unlocked (c, f)
#  define size_t     _IO_size_t
#  define FILE	     _IO_FILE
#else	/* ! USE_IN_LIBIO */
#  define PUT(f, s, n) fwrite (s, 1, n, f)
#  define PAD(f, c, n) __printf_pad (f, c, n)
ssize_t __printf_pad __P ((FILE *, char pad, int n)); /* In vfprintf.c.  */
#endif	/* USE_IN_LIBIO */

/* Macros for doing the actual output.  */

#define outchar(ch)							      \
  do									      \
    {									      \
      register const int outc = (ch);					      \
      if (putc (outc, fp) == EOF)					      \
	return -1;							      \
      ++done;								      \
    } while (0)

#define PRINT(ptr, len)							      \
  do									      \
    {									      \
      register size_t outlen = (len);					      \
      if (len > 20)							      \
	{								      \
	  if (PUT (fp, ptr, outlen) != outlen)				      \
	    return -1;							      \
	  ptr += outlen;						      \
	  done += outlen;						      \
	}								      \
      else								      \
	{								      \
	  while (outlen-- > 0)						      \
	    outchar (*ptr++);						      \
	}								      \
    } while (0)

#define PADN(ch, len)							      \
  do									      \
    {									      \
      if (PAD (fp, ch, len) != len)					      \
	return -1;							      \
      done += len;							      \
    }									      \
  while (0)

/* We use the GNU MP library to handle large numbers.

   An MP variable occupies a varying number of entries in its array.  We keep
   track of this number for efficiency reasons.  Otherwise we would always
   have to process the whole array.  */
#define MPN_VAR(name) mp_limb_t *name; mp_size_t name##size

#define MPN_ASSIGN(dst,src)						      \
  memcpy (dst, src, (dst##size = src##size) * sizeof (mp_limb_t))
#define MPN_GE(u,v) \
  (u##size > v##size || (u##size == v##size && __mpn_cmp (u, v, u##size) >= 0))

extern int __isinfl (long double), __isnanl (long double);

extern mp_size_t __mpn_extract_double (mp_ptr res_ptr, mp_size_t size,
				       int *expt, int *is_neg,
				       double value);
extern mp_size_t __mpn_extract_long_double (mp_ptr res_ptr, mp_size_t size,
					    int *expt, int *is_neg,
					    long double value);
extern unsigned int __guess_grouping (unsigned int intdig_max,
				      const char *grouping, wchar_t sepchar);


static char *group_number (char *buf, char *bufend, unsigned int intdig_no,
			   const char *grouping, wchar_t thousands_sep)
     internal_function;


int
__printf_fp (FILE *fp,
	     const struct printf_info *info,
	     const void *const *args)
{
  /* The floating-point value to output.  */
  union
    {
      double dbl;
      __long_double_t ldbl;
    }
  fpnum;

  /* Locale-dependent representation of decimal point.	*/
  wchar_t decimal;

  /* Locale-dependent thousands separator and grouping specification.  */
  wchar_t thousands_sep;
  const char *grouping;

  /* "NaN" or "Inf" for the special cases.  */
  const char *special = NULL;

  /* We need just a few limbs for the input before shifting to the right
     position.	*/
  mp_limb_t fp_input[(LDBL_MANT_DIG + BITS_PER_MP_LIMB - 1) / BITS_PER_MP_LIMB];
  /* We need to shift the contents of fp_input by this amount of bits.	*/
  int to_shift = 0;

  /* The fraction of the floting-point value in question  */
  MPN_VAR(frac);
  /* and the exponent.	*/
  int exponent;
  /* Sign of the exponent.  */
  int expsign = 0;
  /* Sign of float number.  */
  int is_neg = 0;

  /* Scaling factor.  */
  MPN_VAR(scale);

  /* Temporary bignum value.  */
  MPN_VAR(tmp);

  /* Digit which is result of last hack_digit() call.  */
  int digit;

  /* The type of output format that will be used: 'e'/'E' or 'f'.  */
  int type;

  /* Counter for number of written characters.	*/
  int done = 0;

  /* General helper (carry limb).  */
  mp_limb_t cy;

  char hack_digit (void)
    {
      mp_limb_t hi;

      if (expsign != 0 && type == 'f' && exponent-- > 0)
	hi = 0;
      else if (scalesize == 0)
	{
	  hi = frac[fracsize - 1];
	  cy = __mpn_mul_1 (frac, frac, fracsize - 1, 10);
	  frac[fracsize - 1] = cy;
	}
      else
	{
	  if (fracsize < scalesize)
	    hi = 0;
	  else
	    {
	      hi = mpn_divmod (tmp, frac, fracsize, scale, scalesize);
	      tmp[fracsize - scalesize] = hi;
	      hi = tmp[0];

	      fracsize = scalesize;
	      while (fracsize != 0 && frac[fracsize - 1] == 0)
		--fracsize;
	      if (fracsize == 0)
		{
		  /* We're not prepared for an mpn variable with zero
		     limbs.  */
		  fracsize = 1;
		  return '0' + hi;
		}
	    }

	  cy = __mpn_mul_1 (frac, frac, fracsize, 10);
	  if (cy != 0)
	    frac[fracsize++] = cy;
	}

      return '0' + hi;
    }


  /* Figure out the decimal point character.  */
  if (info->extra == 0)
    {
      if (mbtowc (&decimal, _NL_CURRENT (LC_NUMERIC, DECIMAL_POINT),
		  strlen (_NL_CURRENT (LC_NUMERIC, DECIMAL_POINT))) <= 0)
	decimal = (wchar_t) *_NL_CURRENT (LC_NUMERIC, DECIMAL_POINT);
    }
  else
    {
      if (mbtowc (&decimal, _NL_CURRENT (LC_MONETARY, MON_DECIMAL_POINT),
		  strlen (_NL_CURRENT (LC_MONETARY, MON_DECIMAL_POINT))) <= 0)
	decimal = (wchar_t) *_NL_CURRENT (LC_MONETARY, MON_DECIMAL_POINT);
    }
  /* Give default value.  */
  if (decimal == L'\0')
    decimal = L'.';


  if (info->group)
    {
      if (info->extra == 0)
	grouping = _NL_CURRENT (LC_NUMERIC, GROUPING);
      else
	grouping = _NL_CURRENT (LC_MONETARY, MON_GROUPING);

      if (*grouping <= 0 || *grouping == CHAR_MAX)
	grouping = NULL;
      else
	{
	  /* Figure out the thousands separator character.  */
	  if (info->extra == 0)
	    {
	      if (mbtowc (&thousands_sep, _NL_CURRENT (LC_NUMERIC,
						       THOUSANDS_SEP),
			  strlen (_NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP)))
		  <= 0)
		thousands_sep = (wchar_t) *_NL_CURRENT (LC_NUMERIC,
							THOUSANDS_SEP);
	    }
	  else
	    {
	      if (mbtowc (&thousands_sep, _NL_CURRENT (LC_MONETARY,
						       MON_THOUSANDS_SEP),
			  strlen (_NL_CURRENT (LC_MONETARY,
					       MON_THOUSANDS_SEP))) <= 0)
		thousands_sep = (wchar_t) *_NL_CURRENT (LC_MONETARY,
							MON_THOUSANDS_SEP);
	    }

	  if (thousands_sep == L'\0')
	    grouping = NULL;
	}
    }
  else
    grouping = NULL;

  /* Fetch the argument value.	*/
#ifndef __NO_LONG_DOUBLE_MATH
  if (info->is_long_double && sizeof (long double) > sizeof (double))
    {
      fpnum.ldbl = *(const long double *) args[0];

      /* Check for special values: not a number or infinity.  */
      if (__isnanl (fpnum.ldbl))
	{
	  special = isupper (info->spec) ? "NAN" : "nan";
	  is_neg = 0;
	}
      else if (__isinfl (fpnum.ldbl))
	{
	  special = isupper (info->spec) ? "INF" : "inf";
	  is_neg = fpnum.ldbl < 0;
	}
      else
	{
	  fracsize = __mpn_extract_long_double (fp_input,
						(sizeof (fp_input) /
						 sizeof (fp_input[0])),
						&exponent, &is_neg,
						fpnum.ldbl);
	  to_shift = 1 + fracsize * BITS_PER_MP_LIMB - LDBL_MANT_DIG;
	}
    }
  else
#endif	/* no long double */
    {
      fpnum.dbl = *(const double *) args[0];

      /* Check for special values: not a number or infinity.  */
      if (__isnan (fpnum.dbl))
	{
	  special = isupper (info->spec) ? "NAN" : "nan";
	  is_neg = 0;
	}
      else if (__isinf (fpnum.dbl))
	{
	  special = isupper (info->spec) ? "INF" : "inf";
	  is_neg = fpnum.dbl < 0;
	}
      else
	{
	  fracsize = __mpn_extract_double (fp_input,
					   (sizeof (fp_input)
					    / sizeof (fp_input[0])),
					   &exponent, &is_neg, fpnum.dbl);
	  to_shift = 1 + fracsize * BITS_PER_MP_LIMB - DBL_MANT_DIG;
	}
    }

  if (special)
    {
      int width = info->width;

      if (is_neg || info->showsign || info->space)
	--width;
      width -= 3;

      if (!info->left && width > 0)
	PADN (' ', width);

      if (is_neg)
	outchar ('-');
      else if (info->showsign)
	outchar ('+');
      else if (info->space)
	outchar (' ');

      PRINT (special, 3);

      if (info->left && width > 0)
	PADN (' ', width);

      return done;
    }


  /* We need three multiprecision variables.  Now that we have the exponent
     of the number we can allocate the needed memory.  It would be more
     efficient to use variables of the fixed maximum size but because this
     would be really big it could lead to memory problems.  */
  {
    mp_size_t bignum_size = ((ABS (exponent) + BITS_PER_MP_LIMB - 1)
			     / BITS_PER_MP_LIMB + 4) * sizeof (mp_limb_t);
    frac = (mp_limb_t *) alloca (bignum_size);
    tmp = (mp_limb_t *) alloca (bignum_size);
    scale = (mp_limb_t *) alloca (bignum_size);
  }

  /* We now have to distinguish between numbers with positive and negative
     exponents because the method used for the one is not applicable/efficient
     for the other.  */
  scalesize = 0;
  if (exponent > 2)
    {
      /* |FP| >= 8.0.  */
      int scaleexpo = 0;
      int explog = LDBL_MAX_10_EXP_LOG;
      int exp10 = 0;
      const struct mp_power *tens = &_fpioconst_pow10[explog + 1];
      int cnt_h, cnt_l, i;

      if ((exponent + to_shift) % BITS_PER_MP_LIMB == 0)
	{
	  MPN_COPY_DECR (frac + (exponent + to_shift) / BITS_PER_MP_LIMB,
			 fp_input, fracsize);
	  fracsize += (exponent + to_shift) / BITS_PER_MP_LIMB;
	}
      else
	{
	  cy = __mpn_lshift (frac + (exponent + to_shift) / BITS_PER_MP_LIMB,
			     fp_input, fracsize,
			     (exponent + to_shift) % BITS_PER_MP_LIMB);
	  fracsize += (exponent + to_shift) / BITS_PER_MP_LIMB;
	  if (cy)
	    frac[fracsize++] = cy;
	}
      MPN_ZERO (frac, (exponent + to_shift) / BITS_PER_MP_LIMB);

      assert (tens > &_fpioconst_pow10[0]);
      do
	{
	  --tens;

	  /* The number of the product of two binary numbers with n and m
	     bits respectively has m+n or m+n-1 bits.	*/
	  if (exponent >= scaleexpo + tens->p_expo - 1)
	    {
	      if (scalesize == 0)
		MPN_ASSIGN (tmp, tens->array);
	      else
		{
		  cy = __mpn_mul (tmp, scale, scalesize,
				  &tens->array[_FPIO_CONST_OFFSET],
				  tens->arraysize - _FPIO_CONST_OFFSET);
		  tmpsize = scalesize + tens->arraysize - _FPIO_CONST_OFFSET;
		  if (cy == 0)
		    --tmpsize;
		}

	      if (MPN_GE (frac, tmp))
		{
		  int cnt;
		  MPN_ASSIGN (scale, tmp);
		  count_leading_zeros (cnt, scale[scalesize - 1]);
		  scaleexpo = (scalesize - 2) * BITS_PER_MP_LIMB - cnt - 1;
		  exp10 |= 1 << explog;
		}
	    }
	  --explog;
	}
      while (tens > &_fpioconst_pow10[0]);
      exponent = exp10;

      /* Optimize number representations.  We want to represent the numbers
	 with the lowest number of bytes possible without losing any
	 bytes. Also the highest bit in the scaling factor has to be set
	 (this is a requirement of the MPN division routines).  */
      if (scalesize > 0)
	{
	  /* Determine minimum number of zero bits at the end of
	     both numbers.  */
	  for (i = 0; scale[i] == 0 && frac[i] == 0; i++)
	    ;

	  /* Determine number of bits the scaling factor is misplaced.	*/
	  count_leading_zeros (cnt_h, scale[scalesize - 1]);

	  if (cnt_h == 0)
	    {
	      /* The highest bit of the scaling factor is already set.	So
		 we only have to remove the trailing empty limbs.  */
	      if (i > 0)
		{
		  MPN_COPY_INCR (scale, scale + i, scalesize - i);
		  scalesize -= i;
		  MPN_COPY_INCR (frac, frac + i, fracsize - i);
		  fracsize -= i;
		}
	    }
	  else
	    {
	      if (scale[i] != 0)
		{
		  count_trailing_zeros (cnt_l, scale[i]);
		  if (frac[i] != 0)
		    {
		      int cnt_l2;
		      count_trailing_zeros (cnt_l2, frac[i]);
		      if (cnt_l2 < cnt_l)
			cnt_l = cnt_l2;
		    }
		}
	      else
		count_trailing_zeros (cnt_l, frac[i]);

	      /* Now shift the numbers to their optimal position.  */
	      if (i == 0 && BITS_PER_MP_LIMB - cnt_h > cnt_l)
		{
		  /* We cannot save any memory.	 So just roll both numbers
		     so that the scaling factor has its highest bit set.  */

		  (void) __mpn_lshift (scale, scale, scalesize, cnt_h);
		  cy = __mpn_lshift (frac, frac, fracsize, cnt_h);
		  if (cy != 0)
		    frac[fracsize++] = cy;
		}
	      else if (BITS_PER_MP_LIMB - cnt_h <= cnt_l)
		{
		  /* We can save memory by removing the trailing zero limbs
		     and by packing the non-zero limbs which gain another
		     free one. */

		  (void) __mpn_rshift (scale, scale + i, scalesize - i,
				       BITS_PER_MP_LIMB - cnt_h);
		  scalesize -= i + 1;
		  (void) __mpn_rshift (frac, frac + i, fracsize - i,
				       BITS_PER_MP_LIMB - cnt_h);
		  fracsize -= frac[fracsize - i - 1] == 0 ? i + 1 : i;
		}
	      else
		{
		  /* We can only save the memory of the limbs which are zero.
		     The non-zero parts occupy the same number of limbs.  */

		  (void) __mpn_rshift (scale, scale + (i - 1),
				       scalesize - (i - 1),
				       BITS_PER_MP_LIMB - cnt_h);
		  scalesize -= i;
		  (void) __mpn_rshift (frac, frac + (i - 1),
				       fracsize - (i - 1),
				       BITS_PER_MP_LIMB - cnt_h);
		  fracsize -= frac[fracsize - (i - 1) - 1] == 0 ? i : i - 1;
		}
	    }
	}
    }
  else if (exponent < 0)
    {
      /* |FP| < 1.0.  */
      int exp10 = 0;
      int explog = LDBL_MAX_10_EXP_LOG;
      const struct mp_power *tens = &_fpioconst_pow10[explog + 1];
      mp_size_t used_limbs = fracsize - 1;

      /* Now shift the input value to its right place.	*/
      cy = __mpn_lshift (frac, fp_input, fracsize, to_shift);
      frac[fracsize++] = cy;
      assert (cy == 1 || (frac[fracsize - 2] == 0 && frac[0] == 0));

      expsign = 1;
      exponent = -exponent;

      assert (tens != &_fpioconst_pow10[0]);
      do
	{
	  --tens;

	  if (exponent >= tens->m_expo)
	    {
	      int i, incr, cnt_h, cnt_l;
	      mp_limb_t topval[2];

	      /* The __mpn_mul function expects the first argument to be
		 bigger than the second.  */
	      if (fracsize < tens->arraysize - _FPIO_CONST_OFFSET)
		cy = __mpn_mul (tmp, &tens->array[_FPIO_CONST_OFFSET],
				tens->arraysize - _FPIO_CONST_OFFSET,
				frac, fracsize);
	      else
		cy = __mpn_mul (tmp, frac, fracsize,
				&tens->array[_FPIO_CONST_OFFSET],
				tens->arraysize - _FPIO_CONST_OFFSET);
	      tmpsize = fracsize + tens->arraysize - _FPIO_CONST_OFFSET;
	      if (cy == 0)
		--tmpsize;

	      count_leading_zeros (cnt_h, tmp[tmpsize - 1]);
	      incr = (tmpsize - fracsize) * BITS_PER_MP_LIMB
		     + BITS_PER_MP_LIMB - 1 - cnt_h;

	      assert (incr <= tens->p_expo);

	      /* If we increased the exponent by exactly 3 we have to test
		 for overflow.	This is done by comparing with 10 shifted
		 to the right position.	 */
	      if (incr == exponent + 3)
		{
		  if (cnt_h <= BITS_PER_MP_LIMB - 4)
		    {
		      topval[0] = 0;
		      topval[1]
			= ((mp_limb_t) 10) << (BITS_PER_MP_LIMB - 4 - cnt_h);
		    }
		  else
		    {
		      topval[0] = ((mp_limb_t) 10) << (BITS_PER_MP_LIMB - 4);
		      topval[1] = 0;
		      (void) __mpn_lshift (topval, topval, 2,
					   BITS_PER_MP_LIMB - cnt_h);
		    }
		}

	      /* We have to be careful when multiplying the last factor.
		 If the result is greater than 1.0 be have to test it
		 against 10.0.  If it is greater or equal to 10.0 the
		 multiplication was not valid.  This is because we cannot
		 determine the number of bits in the result in advance.  */
	      if (incr < exponent + 3
		  || (incr == exponent + 3 &&
		      (tmp[tmpsize - 1] < topval[1]
		       || (tmp[tmpsize - 1] == topval[1]
			   && tmp[tmpsize - 2] < topval[0]))))
		{
		  /* The factor is right.  Adapt binary and decimal
		     exponents.	 */
		  exponent -= incr;
		  exp10 |= 1 << explog;

		  /* If this factor yields a number greater or equal to
		     1.0, we must not shift the non-fractional digits down. */
		  if (exponent < 0)
		    cnt_h += -exponent;

		  /* Now we optimize the number representation.	 */
		  for (i = 0; tmp[i] == 0; ++i);
		  if (cnt_h == BITS_PER_MP_LIMB - 1)
		    {
		      MPN_COPY (frac, tmp + i, tmpsize - i);
		      fracsize = tmpsize - i;
		    }
		  else
		    {
		      count_trailing_zeros (cnt_l, tmp[i]);

		      /* Now shift the numbers to their optimal position.  */
		      if (i == 0 && BITS_PER_MP_LIMB - 1 - cnt_h > cnt_l)
			{
			  /* We cannot save any memory.	 Just roll the
			     number so that the leading digit is in a
			     separate limb.  */

			  cy = __mpn_lshift (frac, tmp, tmpsize, cnt_h + 1);
			  fracsize = tmpsize + 1;
			  frac[fracsize - 1] = cy;
			}
		      else if (BITS_PER_MP_LIMB - 1 - cnt_h <= cnt_l)
			{
			  (void) __mpn_rshift (frac, tmp + i, tmpsize - i,
					       BITS_PER_MP_LIMB - 1 - cnt_h);
			  fracsize = tmpsize - i;
			}
		      else
			{
			  /* We can only save the memory of the limbs which
			     are zero.	The non-zero parts occupy the same
			     number of limbs.  */

			  (void) __mpn_rshift (frac, tmp + (i - 1),
					       tmpsize - (i - 1),
					       BITS_PER_MP_LIMB - 1 - cnt_h);
			  fracsize = tmpsize - (i - 1);
			}
		    }
		  used_limbs = fracsize - 1;
		}
	    }
	  --explog;
	}
      while (tens != &_fpioconst_pow10[1] && exponent > 0);
      /* All factors but 10^-1 are tested now.	*/
      if (exponent > 0)
	{
	  int cnt_l;

	  cy = __mpn_mul_1 (tmp, frac, fracsize, 10);
	  tmpsize = fracsize;
	  assert (cy == 0 || tmp[tmpsize - 1] < 20);

	  count_trailing_zeros (cnt_l, tmp[0]);
	  if (cnt_l < MIN (4, exponent))
	    {
	      cy = __mpn_lshift (frac, tmp, tmpsize,
				 BITS_PER_MP_LIMB - MIN (4, exponent));
	      if (cy != 0)
		frac[tmpsize++] = cy;
	    }
	  else
	    (void) __mpn_rshift (frac, tmp, tmpsize, MIN (4, exponent));
	  fracsize = tmpsize;
	  exp10 |= 1;
	  assert (frac[fracsize - 1] < 10);
	}
      exponent = exp10;
    }
  else
    {
      /* This is a special case.  We don't need a factor because the
	 numbers are in the range of 0.0 <= fp < 8.0.  We simply
	 shift it to the right place and divide it by 1.0 to get the
	 leading digit.	 (Of course this division is not really made.)	*/
      assert (0 <= exponent && exponent < 3 &&
	      exponent + to_shift < BITS_PER_MP_LIMB);

      /* Now shift the input value to its right place.	*/
      cy = __mpn_lshift (frac, fp_input, fracsize, (exponent + to_shift));
      frac[fracsize++] = cy;
      exponent = 0;
    }

  {
    int width = info->width;
    char *buffer, *startp, *cp;
    int chars_needed;
    int expscale;
    int intdig_max, intdig_no = 0;
    int fracdig_min, fracdig_max, fracdig_no = 0;
    int dig_max;
    int significant;

    if (tolower (info->spec) == 'e')
      {
	type = info->spec;
	intdig_max = 1;
	fracdig_min = fracdig_max = info->prec < 0 ? 6 : info->prec;
	chars_needed = 1 + 1 + fracdig_max + 1 + 1 + 4;
	/*	       d   .	 ddd	     e	 +-  ddd  */
	dig_max = INT_MAX;		/* Unlimited.  */
	significant = 1;		/* Does not matter here.  */
      }
    else if (info->spec == 'f')
      {
	type = 'f';
	fracdig_min = fracdig_max = info->prec < 0 ? 6 : info->prec;
	if (expsign == 0)
	  {
	    intdig_max = exponent + 1;
	    /* This can be really big!	*/  /* XXX Maybe malloc if too big? */
	    chars_needed = exponent + 1 + 1 + fracdig_max;
	  }
	else
	  {
	    intdig_max = 1;
	    chars_needed = 1 + 1 + fracdig_max;
	  }
	dig_max = INT_MAX;		/* Unlimited.  */
	significant = 1;		/* Does not matter here.  */
      }
    else
      {
	dig_max = info->prec < 0 ? 6 : (info->prec == 0 ? 1 : info->prec);
	if ((expsign == 0 && exponent >= dig_max)
	    || (expsign != 0 && exponent > 4))
	  {
	    type = isupper (info->spec) ? 'E' : 'e';
	    fracdig_max = dig_max - 1;
	    intdig_max = 1;
	    chars_needed = 1 + 1 + fracdig_max + 1 + 1 + 4;
	  }
	else
	  {
	    type = 'f';
	    intdig_max = expsign == 0 ? exponent + 1 : 0;
	    fracdig_max = dig_max - intdig_max;
	    /* We need space for the significant digits and perhaps for
	       leading zeros when < 1.0.  Pessimistic guess: dig_max.  */
	    chars_needed = dig_max + dig_max + 1;
	  }
	fracdig_min = info->alt ? fracdig_max : 0;
	significant = 0;		/* We count significant digits.	 */
      }

    if (grouping)
      /* Guess the number of groups we will make, and thus how
	 many spaces we need for separator characters.  */
      chars_needed += __guess_grouping (intdig_max, grouping, thousands_sep);

    /* Allocate buffer for output.  We need two more because while rounding
       it is possible that we need two more characters in front of all the
       other output.  */
    buffer = alloca (2 + chars_needed);
    cp = startp = buffer + 2;	/* Let room for rounding.  */

    /* Do the real work: put digits in allocated buffer.  */
    if (expsign == 0 || type != 'f')
      {
	assert (expsign == 0 || intdig_max == 1);
	while (intdig_no < intdig_max)
	  {
	    ++intdig_no;
	    *cp++ = hack_digit ();
	  }
	significant = 1;
	if (info->alt
	    || fracdig_min > 0
	    || (fracdig_max > 0 && (fracsize > 1 || frac[0] != 0)))
	  *cp++ = decimal;
      }
    else
      {
	/* |fp| < 1.0 and the selected type is 'f', so put "0."
	   in the buffer.  */
	*cp++ = '0';
	--exponent;
	*cp++ = decimal;
      }

    /* Generate the needed number of fractional digits.	 */
    while (fracdig_no < fracdig_min
	   || (fracdig_no < fracdig_max && (fracsize > 1 || frac[0] != 0)))
      {
	++fracdig_no;
	*cp = hack_digit ();
	if (*cp != '0')
	  significant = 1;
	else if (significant == 0)
	  {
	    ++fracdig_max;
	    if (fracdig_min > 0)
	      ++fracdig_min;
	  }
	++cp;
      }

    /* Do rounding.  */
    digit = hack_digit ();
    if (digit > '4')
      {
	char *tp = cp;

	if (digit == '5' && (*(cp - 1) & 1) == 0)
	  {
	    /* This is the critical case.	 */
	    if (fracsize == 1 && frac[0] == 0)
	      /* Rest of the number is zero -> round to even.
		 (IEEE 754-1985 4.1 says this is the default rounding.)  */
	      goto do_expo;
	    else if (scalesize == 0)
	      {
		/* Here we have to see whether all limbs are zero since no
		   normalization happened.  */
		size_t lcnt = fracsize;
		while (lcnt >= 1 && frac[lcnt - 1] == 0)
		  --lcnt;
		if (lcnt == 0)
		  /* Rest of the number is zero -> round to even.
		     (IEEE 754-1985 4.1 says this is the default rounding.)  */
		  goto do_expo;
	      }
	  }

	if (fracdig_no > 0)
	  {
	    /* Process fractional digits.  Terminate if not rounded or
	       radix character is reached.  */
	    while (*--tp != decimal && *tp == '9')
	      *tp = '0';
	    if (*tp != decimal)
	      /* Round up.  */
	      (*tp)++;
	  }

	if (fracdig_no == 0 || *tp == decimal)
	  {
	    /* Round the integer digits.  */
	    if (*(tp - 1) == decimal)
	      --tp;

	    while (--tp >= startp && *tp == '9')
	      *tp = '0';

	    if (tp >= startp)
	      /* Round up.  */
	      (*tp)++;
	    else
	      /* It is more critical.  All digits were 9's.  */
	      {
		if (type != 'f')
		  {
		    *startp = '1';
		    exponent += expsign == 0 ? 1 : -1;
		  }
		else if (intdig_no == dig_max)
		  {
		    /* This is the case where for type %g the number fits
		       really in the range for %f output but after rounding
		       the number of digits is too big.	 */
		    *--startp = decimal;
		    *--startp = '1';

		    if (info->alt || fracdig_no > 0)
		      {
			/* Overwrite the old radix character.  */
			startp[intdig_no + 2] = '0';
			++fracdig_no;
		      }

		    fracdig_no += intdig_no;
		    intdig_no = 1;
		    fracdig_max = intdig_max - intdig_no;
		    ++exponent;
		    /* Now we must print the exponent.	*/
		    type = isupper (info->spec) ? 'E' : 'e';
		  }
		else
		  {
		    /* We can simply add another another digit before the
		       radix.  */
		    *--startp = '1';
		    ++intdig_no;
		  }

		/* While rounding the number of digits can change.
		   If the number now exceeds the limits remove some
		   fractional digits.  */
		if (intdig_no + fracdig_no > dig_max)
		  {
		    cp -= intdig_no + fracdig_no - dig_max;
		    fracdig_no -= intdig_no + fracdig_no - dig_max;
		  }
	      }
	  }
      }

  do_expo:
    /* Now remove unnecessary '0' at the end of the string.  */
    while (fracdig_no > fracdig_min && *(cp - 1) == '0')
      {
	--cp;
	--fracdig_no;
      }
    /* If we eliminate all fractional digits we perhaps also can remove
       the radix character.  */
    if (fracdig_no == 0 && !info->alt && *(cp - 1) == decimal)
      --cp;

    if (grouping)
      /* Add in separator characters, overwriting the same buffer.  */
      cp = group_number (startp, cp, intdig_no, grouping, thousands_sep);

    /* Write the exponent if it is needed.  */
    if (type != 'f')
      {
	*cp++ = type;
	*cp++ = expsign ? '-' : '+';

	/* Find the magnitude of the exponent.	*/
	expscale = 10;
	while (expscale <= exponent)
	  expscale *= 10;

	if (exponent < 10)
	  /* Exponent always has at least two digits.  */
	  *cp++ = '0';
	else
	  do
	    {
	      expscale /= 10;
	      *cp++ = '0' + (exponent / expscale);
	      exponent %= expscale;
	    }
	  while (expscale > 10);
	*cp++ = '0' + exponent;
      }

    /* Compute number of characters which must be filled with the padding
       character.  */
    if (is_neg || info->showsign || info->space)
      --width;
    width -= cp - startp;

    if (!info->left && info->pad != '0' && width > 0)
      PADN (info->pad, width);

    if (is_neg)
      outchar ('-');
    else if (info->showsign)
      outchar ('+');
    else if (info->space)
      outchar (' ');

    if (!info->left && info->pad == '0' && width > 0)
      PADN ('0', width);

    PRINT (startp, cp - startp);

    if (info->left && width > 0)
      PADN (info->pad, width);
  }
  return done;
}

/* Return the number of extra grouping characters that will be inserted
   into a number with INTDIG_MAX integer digits.  */

unsigned int
__guess_grouping (unsigned int intdig_max, const char *grouping,
		  wchar_t sepchar)
{
  unsigned int groups;

  /* We treat all negative values like CHAR_MAX.  */

  if (*grouping == CHAR_MAX || *grouping <= 0)
    /* No grouping should be done.  */
    return 0;

  groups = 0;
  while (intdig_max > (unsigned int) *grouping)
    {
      ++groups;
      intdig_max -= *grouping++;

      if (*grouping == CHAR_MAX
#if CHAR_MIN < 0
	  || *grouping < 0
#endif
	  )
	/* No more grouping should be done.  */
	break;
      else if (*grouping == 0)
	{
	  /* Same grouping repeats.  */
	  groups += (intdig_max - 1) / grouping[-1];
	  break;
	}
    }

  return groups;
}

/* Group the INTDIG_NO integer digits of the number in [BUF,BUFEND).
   There is guaranteed enough space past BUFEND to extend it.
   Return the new end of buffer.  */

static char *
internal_function
group_number (char *buf, char *bufend, unsigned int intdig_no,
	      const char *grouping, wchar_t thousands_sep)
{
  unsigned int groups = __guess_grouping (intdig_no, grouping, thousands_sep);
  char *p;

  if (groups == 0)
    return bufend;

  /* Move the fractional part down.  */
  memmove (buf + intdig_no + groups, buf + intdig_no,
	   bufend - (buf + intdig_no));

  p = buf + intdig_no + groups - 1;
  do
    {
      unsigned int len = *grouping++;
      do
	*p-- = buf[--intdig_no];
      while (--len > 0);
      *p-- = thousands_sep;

      if (*grouping == CHAR_MAX
#if CHAR_MIN < 0
	  || *grouping < 0
#endif
	  )
	/* No more grouping should be done.  */
	break;
      else if (*grouping == 0)
	/* Same grouping repeats.  */
	--grouping;
    } while (intdig_no > (unsigned int) *grouping);

  /* Copy the remaining ungrouped digits.  */
  do
    *p-- = buf[--intdig_no];
  while (p > buf);

  return bufend + groups;
}
