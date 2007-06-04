/* Print floating point number in hexadecimal notation according to ISO C99.
   Copyright (C) 1997,1998,1999,2000,2001,2002,2004,2006,2007
	Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#define PRINT_FPHEX_LONG_DOUBLE \
do {									      \
      /* We have 105 bits of mantissa plus one implicit digit.  Since	      \
	 106 bits are representable without rest using hexadecimal	      \
	 digits we use only the implicit digits for the number before	      \
	 the decimal point.  */						      \
      unsigned long long int num0, num1;				      \
      unsigned long long hi, lo;					      \
      int ediff;							      \
      union ibm_extended_long_double eldbl;				      \
      eldbl.d = fpnum.ldbl.d;						      \
									      \
      assert (sizeof (long double) == 16);				      \
									      \
      lo = ((long long)eldbl.ieee.mantissa2 << 32) | eldbl.ieee.mantissa3;    \
      hi = ((long long)eldbl.ieee.mantissa0 << 32) | eldbl.ieee.mantissa1;    \
      lo <<= 7; /* pre-shift lo to match ieee854.  */			      \
      /* If the lower double is not a denomal or zero then set the hidden     \
	 53rd bit.  */							      \
      if (eldbl.ieee.exponent2 != 0)					      \
	lo |= (1ULL << (52 + 7));					      \
      else								      \
	lo <<= 1;							      \
      /* The lower double is normalized separately from the upper.  We	      \
	 may need to adjust the lower manitissa to reflect this.  */	      \
      ediff = eldbl.ieee.exponent - eldbl.ieee.exponent2;		      \
      if (ediff > 53 + 63)						      \
	lo = 0;								      \
      else if (ediff > 53)						      \
	lo = lo >> (ediff - 53);					      \
      else if (eldbl.ieee.exponent2 == 0 && ediff < 53)			      \
	lo = lo << (53 - ediff);					      \
      if (eldbl.ieee.negative != eldbl.ieee.negative2			      \
	  && (eldbl.ieee.exponent2 != 0 || lo != 0L))			      \
	{								      \
	  lo = (1ULL << 60) - lo;					      \
	  if (hi == 0L)							      \
	    {								      \
	      /* we have a borrow from the hidden bit, so shift left 1.  */   \
	      hi = 0xffffffffffffeLL | (lo >> 59);			      \
	      lo = 0xfffffffffffffffLL & (lo << 1);			      \
	      eldbl.ieee.exponent--;					      \
	    }								      \
	  else								      \
	    hi--;							      \
        }								      \
      num1 = (hi << 60) | lo;						      \
      num0 = hi >> 4;							      \
									      \
      zero_mantissa = (num0|num1) == 0;					      \
									      \
      if (sizeof (unsigned long int) > 6)				      \
	{								      \
	  numstr = _itoa_word (num1, numbuf + sizeof numbuf, 16,	      \
			       info->spec == 'A');			      \
	  wnumstr = _itowa_word (num1,					      \
				 wnumbuf + sizeof (wnumbuf) / sizeof (wchar_t),\
				 16, info->spec == 'A');		      \
	}								      \
      else								      \
	{								      \
	  numstr = _itoa (num1, numbuf + sizeof numbuf, 16,		      \
			  info->spec == 'A');				      \
	  wnumstr = _itowa (num1,					      \
			    wnumbuf + sizeof (wnumbuf) / sizeof (wchar_t),    \
			    16, info->spec == 'A');			      \
	}								      \
									      \
      while (numstr > numbuf + (sizeof numbuf - 64 / 4))		      \
	{								      \
	  *--numstr = '0';						      \
	  *--wnumstr = L'0';						      \
	}								      \
									      \
      if (sizeof (unsigned long int) > 6)				      \
	{								      \
	  numstr = _itoa_word (num0, numstr, 16, info->spec == 'A');	      \
	  wnumstr = _itowa_word (num0, wnumstr, 16, info->spec == 'A');	      \
	}								      \
      else								      \
	{								      \
	  numstr = _itoa (num0, numstr, 16, info->spec == 'A');		      \
	  wnumstr = _itowa (num0, wnumstr, 16, info->spec == 'A');	      \
	}								      \
									      \
      /* Fill with zeroes.  */						      \
      while (numstr > numbuf + (sizeof numbuf - 112 / 4))		      \
	{								      \
	  *--numstr = '0';						      \
	  *--wnumstr = L'0';						      \
	}								      \
									      \
      leading = eldbl.ieee.exponent == 0 ? '0' : '1';			      \
									      \
      exponent = eldbl.ieee.exponent;					      \
									      \
      if (exponent == 0)						      \
	{								      \
	  if (zero_mantissa)						      \
	    expnegative = 0;						      \
	  else								      \
	    {								      \
	      /* This is a denormalized number.  */			      \
	      expnegative = 1;						      \
	      exponent = IBM_EXTENDED_LONG_DOUBLE_BIAS - 1;		      \
	    }								      \
	}								      \
      else if (exponent >= IBM_EXTENDED_LONG_DOUBLE_BIAS)		      \
	{								      \
	  expnegative = 0;						      \
	  exponent -= IBM_EXTENDED_LONG_DOUBLE_BIAS;			      \
	}								      \
      else								      \
	{								      \
	  expnegative = 1;						      \
	  exponent = -(exponent - IBM_EXTENDED_LONG_DOUBLE_BIAS);	      \
	}								      \
} while (0)

#include <stdio-common/printf_fphex.c>
