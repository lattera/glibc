/* Internal function for converting integers to ASCII.
   Copyright (C) 1994-1999,2002,2003,2007 Free Software Foundation, Inc.
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

#ifndef _ITOA_H
#define _ITOA_H

#include <limits.h>

/* Convert VALUE into ASCII in base BASE (2..36).
   Write backwards starting the character just before BUFLIM.
   Return the address of the first (left-to-right) character in the number.
   Use upper case letters iff UPPER_CASE is nonzero.  */

extern char *_itoa (unsigned long long int value, char *buflim,
		    unsigned int base, int upper_case);

extern const char _itoa_upper_digits[];
extern const char _itoa_upper_digits_internal[] attribute_hidden;
extern const char _itoa_lower_digits[];
extern const char _itoa_lower_digits_internal[] attribute_hidden;

#ifndef NOT_IN_libc
extern char *_itoa_word (unsigned long value, char *buflim,
			 unsigned int base, int upper_case);
#else
static inline char * __attribute__ ((unused, always_inline))
_itoa_word (unsigned long value, char *buflim,
	    unsigned int base, int upper_case)
{
  const char *digits = (upper_case
# if defined IS_IN_rtld
			? INTUSE(_itoa_upper_digits)
			: INTUSE(_itoa_lower_digits)
# else
			? _itoa_upper_digits
			: _itoa_lower_digits
# endif
		       );

  switch (base)
    {
# define SPECIAL(Base)							      \
    case Base:								      \
      do								      \
	*--buflim = digits[value % Base];				      \
      while ((value /= Base) != 0);					      \
      break

      SPECIAL (10);
      SPECIAL (16);
      SPECIAL (8);
    default:
      do
	*--buflim = digits[value % base];
      while ((value /= base) != 0);
    }
  return buflim;
}
# undef SPECIAL
#endif

/* Similar to the _itoa functions, but output starts at buf and pointer
   after the last written character is returned.  */
extern char *_fitoa_word (unsigned long value, char *buf, unsigned int base,
			  int upper_case) attribute_hidden;
extern char *_fitoa (unsigned long long value, char *buf, unsigned int base,
		     int upper_case) attribute_hidden;

#if LONG_MAX == LLONG_MAX
/* No need for special long long versions.  */
# define _itoa(value, buf, base, upper_case) \
  _itoa_word (value, buf, base, upper_case)
# define _fitoa(value, buf, base, upper_case) \
  _fitoa_word (value, buf, base, upper_case)
#endif

#endif	/* itoa.h */
