/* Print floating point number in hexadecimal notation according to
   ISO C 9X.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <ctype.h>
#include <ieee754.h>
#include <math.h>
#include <printf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "_itoa.h"
#include <locale/localeinfo.h>

/* #define NDEBUG 1*/		/* Undefine this for debugging assertions.  */
#include <assert.h>

/* This defines make it possible to use the same code for GNU C library and
   the GNU I/O library.	 */
#ifdef USE_IN_LIBIO
# include <libioP.h>
# define PUT(f, s, n) _IO_sputn (f, s, n)
# define PAD(f, c, n) _IO_padn (f, c, n)
/* We use this file GNU C library and GNU I/O library.	So make
   names equal.	 */
# undef putc
# define putc(c, f) _IO_putc_unlocked (c, f)
# define size_t     _IO_size_t
# define FILE	     _IO_FILE
#else	/* ! USE_IN_LIBIO */
# define PUT(f, s, n) fwrite (s, 1, n, f)
# define PAD(f, c, n) __printf_pad (f, c, n)
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
       int outlen = (len);						      \
       const char *cp = (ptr);						      \
       while (outlen-- > 0)						      \
	 outchar (*cp++);						      \
    } while (0)

#define PADN(ch, len)							      \
  do									      \
    {									      \
      if (PAD (fp, ch, len) != len)					      \
	return -1;							      \
      done += len;							      \
    }									      \
  while (0)

#ifndef MIN
# define MIN(a,b) ((a)<(b)?(a):(b))
#endif


int
__printf_fphex (FILE *fp,
		const struct printf_info *info,
		const void *const *args)
{
  /* The floating-point value to output.  */
  union
    {
      union ieee754_double dbl;
      union ieee854_long_double ldbl;
    }
  fpnum;

  /* Locale-dependent representation of decimal point.	*/
  wchar_t decimal;

  /* "NaN" or "Inf" for the special cases.  */
  const char *special = NULL;

  /* Buffer for the generated number string for the mantissa.  The
     maximal size for the mantissa is 64 bits.  */
  char numbuf[16];
  char *numstr;
  char *numend;
  int negative;

  /* The maximal exponent of two in decimal notation has 5 digits.  */
  char expbuf[5];
  char *expstr;
  int expnegative;
  int exponent;

  /* Non-zero is mantissa is zero.  */
  int zero_mantissa;

  /* The leading digit before the decimal point.  */
  char leading;

  /* Precision.  */
  int precision = info->prec;

  /* Width.  */
  int width = info->width;

  /* Number of characters written.  */
  int done = 0;


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


  /* Fetch the argument value.	*/
  if (info->is_long_double && sizeof (long double) > sizeof (double))
    {
      fpnum.ldbl.d = *(const long double *) args[0];

      /* Check for special values: not a number or infinity.  */
      if (__isnanl (fpnum.ldbl.d))
	{
	  special = isupper (info->spec) ? "NAN" : "nan";
	  negative = 0;
	}
      else
	{
	  if (__isinfl (fpnum.ldbl.d))
	    special = isupper (info->spec) ? "INF" : "inf";

	  negative = signbit (fpnum.ldbl.d);
	}
    }
  else
    {
      fpnum.dbl.d = *(const double *) args[0];

      /* Check for special values: not a number or infinity.  */
      if (__isnan (fpnum.dbl.d))
	{
	  special = isupper (info->spec) ? "NAN" : "nan";
	  negative = 0;
	}
      else
	{
	  if (__isinf (fpnum.dbl.d))
	    special = isupper (info->spec) ? "INF" : "inf";

	  negative = signbit (fpnum.dbl.d);
	}
    }

  if (special)
    {
      int width = info->width;

      if (negative || info->showsign || info->space)
	--width;
      width -= 3;

      if (!info->left && width > 0)
	PADN (' ', width);

      if (negative)
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

  /* We are handling here only 64 and 80 bit IEEE foating point
     numbers.  */
  if (info->is_long_double == 0 || sizeof (double) == sizeof (long double))
    {
      /* We have 52 bits of mantissa plus one implicit digit.  Since
	 52 bits are representable without rest using hexadecimal
	 digits we use only the implicit digits for the number before
	 the decimal point.  */
      unsigned long long int num;

      num = (((unsigned long long int) fpnum.dbl.ieee.mantissa0) << 32
	     | fpnum.dbl.ieee.mantissa1);

      zero_mantissa = num == 0;

      if (sizeof (unsigned long int) > 6)
	numstr = _itoa_word (num, numbuf + sizeof numbuf, 16,
			     info->spec == 'A');
      else
	numstr = _itoa (num, numbuf + sizeof numbuf, 16,
			info->spec == 'A');

      /* Fill with zeroes.  */
      while (numstr > numbuf + (sizeof numbuf - 52 / 4))
	*--numstr = '0';

      leading = fpnum.dbl.ieee.exponent == 0 ? '0' : '1';

      exponent = fpnum.dbl.ieee.exponent;

      if (exponent == 0)
	{
	  if (zero_mantissa)
	    expnegative = 0;
	  else
	    {
	      /* This is a denormalized number.  */
	      expnegative = 1;
	      exponent = -(1 - IEEE754_DOUBLE_BIAS);
	    }
	}
      else if (exponent >= IEEE754_DOUBLE_BIAS)
	{
	  expnegative = 0;
	  exponent -= IEEE754_DOUBLE_BIAS;
	}
      else
	{
	  expnegative = 1;
	  exponent = -(exponent - IEEE754_DOUBLE_BIAS);
	}
    }
  else
    {
      /* The "strange" 80 bit format on ix86 and m68k has an explicit
	 leading digit in the 64 bit mantissa.  */
      unsigned long long int num;

      assert (sizeof (long double) == 12);

      num = (((unsigned long long int) fpnum.ldbl.ieee.mantissa0) << 32
	     | fpnum.ldbl.ieee.mantissa1);

      zero_mantissa = num == 0;

      if (sizeof (unsigned long int) > 6)
	numstr = _itoa_word (num, numbuf + sizeof numbuf, 16,
			     info->spec == 'A');
      else
	numstr = _itoa (num, numbuf + sizeof numbuf, 16, info->spec == 'A');

      /* Fill with zeroes.  */
      while (numstr > numbuf + (sizeof numbuf - 64 / 4))
	*--numstr = '0';

      /* We use a full nibble for the leading digit.  */
      leading = *numstr++;

      /* We have 3 bits from the mantissa in the leading nibble.
	 Therefore we are here using `IEEE854_LONG_DOUBLE_BIAS + 3'.  */
      exponent = fpnum.ldbl.ieee.exponent;

      if (exponent == 0)
	{
	  if (zero_mantissa)
	    expnegative = 0;
	  else
	    {
	      /* This is a denormalized number.  */
	      expnegative = 1;
	      exponent = -(1 - (IEEE854_LONG_DOUBLE_BIAS + 3));
	    }
	}
      else if (exponent >= IEEE854_LONG_DOUBLE_BIAS + 3)
	{
	  expnegative = 0;
	  exponent -= IEEE854_LONG_DOUBLE_BIAS + 2;
	}
      else
	{
	  expnegative = 1;
	  exponent = -(exponent - (IEEE854_LONG_DOUBLE_BIAS + 3));
	}
    }

  /* Look for trailing zeroes.  */
  if (! zero_mantissa)
    {
      numend = numbuf + sizeof numbuf;
      while (numend[-1] == '0')
	--numend;

      if (precision == -1)
	precision = numend - numstr;
      else if (precision < numend - numstr
	       && (numstr[precision] > '8'
		   || (('A' < '0' || 'a' < '0')
		       && numstr[precision] < '0')
		   || (numstr[precision] == '8'
		       && (precision + 1 < numend - numstr
			   /* Round to even.  */
			   || (precision > 0
			       && ((numstr[precision - 1] & 1)
				   ^ (isdigit (numstr[precision - 1]) == 0)))
			   || (precision == 0
			       && ((leading & 1)
				   ^ (isdigit (leading) == 0)))))))
	{
	  /* Round up.  */
	  int cnt = precision;
	  while (--cnt >= 0)
	    {
	      char ch = numstr[cnt];
	      /* We assume that the digits and the letters are ordered
		 like in ASCII.  This is true for the rest of GNU, too.  */
	      if (ch == '9')
		{
		  numstr[cnt] = info->spec;	/* This is tricky,
						   think about it!  */
		  break;
		}
	      else if (tolower (ch) < 'f')
		{
		  ++numstr[cnt];
		  break;
		}
	      else
		numstr[cnt] = '0';
	    }
	  if (cnt < 0)
	    {
	      /* The mantissa so far was fff...f  Now increment the
		 leading digit.  Here it is again possible that we
		 get an overflow.  */
	      if (leading == '9')
		leading = info->spec;
	      else if (tolower (leading) < 'f')
		++leading;
	      else
		{
		  leading = 1;
		  if (expnegative)
		    {
		      exponent += 4;
		      if (exponent >= 0)
			expnegative = 0;
		    }
		  else
		    exponent += 4;
		}
	    }
	}
    }
  else
    numend = numstr;

  /* Now we can compute the exponent string.  */
  expstr = _itoa_word (exponent, expbuf + sizeof expbuf, 10, 0);

  /* Now we have all information to compute the size.  */
  width -= ((negative || info->showsign || info->space)
	    /* Sign.  */
	    + 2    + 1 + 1 + precision + 1 + 1
	    /* 0x    h   .   hhh         P   ExpoSign.  */
	    + ((expbuf + sizeof expbuf) - expstr));
	    /* Exponent.  */

  /* A special case if when the mantissa is zero and the `#' is not
     given.  In this case we must not print the decimal point.  */
  if (zero_mantissa && precision == 0 && !info->alt)
    ++width;		/* This nihilates the +1 for the decimal-point
			   character in the following equation.  */

  if (!info->left && width > 0)
    PADN (' ', width);

  if (negative)
    outchar ('-');
  else if (info->showsign)
    outchar ('+');
  else if (info->space)
    outchar (' ');

  outchar ('0');
  outchar (info->spec == 'A' ? 'X' : 'x');
  outchar (leading);

  if (!zero_mantissa || precision > 0 || info->alt)
    outchar (decimal);

  if (!zero_mantissa || precision > 0)
    {
      PRINT (numstr, MIN (numend - numstr, precision));
      if (precision > numend - numstr)
	PADN ('0', precision - (numend - numstr));
    }

  if (info->left && info->pad == '0' && width > 0)
    PADN ('0', width);

  outchar (info->spec == 'A' ? 'P' : 'p');

  outchar (expnegative ? '-' : '+');

  PRINT (expstr, (expbuf + sizeof expbuf) - expstr);

  if (info->left && info->pad != '0' && width > 0)
    PADN (info->pad, width);

  return done;
}
