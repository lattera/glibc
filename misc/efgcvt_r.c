/* [efg]cvt -- compatibility functions for floating point formatting,
   reentrant versions.
Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#ifndef FLOAT_TYPE
#define FLOAT_TYPE double
#define FUNC_PREFIX
#define FLOAT_FMT_FLAG
#define FLOAT_NAME_EXT
#endif

#define APPEND(a, b) APPEND2 (a, b)
#define APPEND2(a, b) a##b

#define FLOOR APPEND(floor, FLOAT_NAME_EXT)
#define FABS APPEND(fabs, FLOAT_NAME_EXT)
#define LOG10 APPEND(log10, FLOAT_NAME_EXT)


int
APPEND (FUNC_PREFIX, fcvt_r) (value, ndigit, decpt, sign, buf, len)
     FLOAT_TYPE value;
     int ndigit, *decpt, *sign;
     char *buf;
     size_t len;
{
  int n, i;

  if (buf == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  *sign = value < 0.0;
  if (*sign)
    value = - value;

  n = snprintf (buf, len, "%.*" FLOAT_FMT_FLAG "f", ndigit, value);
  if (n < 0)
    return -1;

  i = 0;
  while (i < n && isdigit (buf[i]))
    ++i;
  *decpt = i;
  do
    ++i;
  while (! isdigit (buf[i]));
  memmove (&buf[i - *decpt], buf, n - (i - *decpt));

  return 0;
}

#define weak_extern2(name) weak_extern (name)
weak_extern2 (FLOOR) weak_extern2 (LOG10) weak_extern2 (FABS)

int
APPEND (FUNC_PREFIX, ecvt_r) (value, ndigit, decpt, sign, buf, len)
     FLOAT_TYPE value;
     int ndigit, *decpt, *sign;
     char *buf;
     size_t len;
{
  FLOAT_TYPE (*log10_function) (FLOAT_TYPE) = &LOG10;

  if (log10_function)
    {
      /* Use the reasonable code if -lm is included.  */
      ndigit -= (int) FLOOR (LOG10 (FABS (value)));
      if (ndigit < 0)
	ndigit = 0;
    }
  else
    {
      /* Slow code that doesn't require -lm functions.  */
      FLOAT_TYPE d;
      for (d = value < 0.0 ? - value : value;
	   ndigit > 0 && d >= 10.0;
	   d *= 0.1)
	--ndigit;
    }

  return APPEND (FUNC_PREFIX, fcvt_r) (value, ndigit, decpt, sign, buf, len);
}
