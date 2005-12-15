/* Print floating point number in hexadecimal notation according to
   ISO C99.
   Copyright (C) 1997, 1998, 1999, 2000, 2005 Free Software Foundation, Inc.
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

#define PRINT_FPHEX_LONG_DOUBLE \
do {									      \
      /* We have 112 bits of mantissa plus one implicit digit.  Since	      \
	 112 bits are representable without rest using hexadecimal	      \
	 digits we use only the implicit digits for the number before	      \
	 the decimal point.  */						      \
      unsigned long long int num0, num1;				      \
									      \
      assert (sizeof (long double) == 16);				      \
									      \
      num0 = (((unsigned long long int) fpnum.ldbl.ieee.mantissa0) << 32      \
	     | fpnum.ldbl.ieee.mantissa1);				      \
      num1 = (((unsigned long long int) fpnum.ldbl.ieee.mantissa2) << 32      \
	     | fpnum.ldbl.ieee.mantissa3);				      \
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
      leading = fpnum.ldbl.ieee.exponent == 0 ? '0' : '1';		      \
									      \
      exponent = fpnum.ldbl.ieee.exponent;				      \
									      \
      if (exponent == 0)						      \
	{								      \
	  if (zero_mantissa)						      \
	    expnegative = 0;						      \
	  else								      \
	    {								      \
	      /* This is a denormalized number.  */			      \
	      expnegative = 1;						      \
	      exponent = IEEE854_LONG_DOUBLE_BIAS - 1;			      \
	    }								      \
	}								      \
      else if (exponent >= IEEE854_LONG_DOUBLE_BIAS)			      \
	{								      \
	  expnegative = 0;						      \
	  exponent -= IEEE854_LONG_DOUBLE_BIAS;				      \
	}								      \
      else								      \
	{								      \
	  expnegative = 1;						      \
	  exponent = -(exponent - IEEE854_LONG_DOUBLE_BIAS);		      \
	}								      \
} while (0)

#include <stdio-common/printf_fphex.c>
