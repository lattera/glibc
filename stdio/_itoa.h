/* Internal function for converting integers to ASCII.
Copyright (C) 1994 Free Software Foundation, Inc.
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

#ifndef _ITOA_H
#define _ITOA_H
#include <sys/cdefs.h>

extern const char _itoa_lower_digits[], _itoa_upper_digits[];

/* Convert VALUE into ASCII in base BASE (2..36).
   Write backwards starting the character just before BUFLIM.
   Return the address of the first (left-to-right) character in the number.
   Use upper case letters iff UPPER_CASE is nonzero.  */

extern char *_itoa __P ((unsigned long long int value, char *buflim,
			 unsigned int base, int upper_case));

#if defined (__GNUC__) && defined (__OPTIMIZE__)
extern __inline char *
_itoa (unsigned long long int value, char *buflim,
       unsigned int base, int upper_case)
{
  /* Base-36 digits for numbers.  */
  const char *digits = upper_case ? _itoa_upper_digits : _itoa_lower_digits;

  register char *bp = buflim;

  while (value > 0)
    {
      *--bp = digits[value % base];
      value /= base;
    }

  return bp;
}
#endif

#endif	/* itoa.h */
