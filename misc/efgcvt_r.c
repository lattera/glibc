/* [efg]cvt -- compatibility functions for floating point formatting,
   reentrent versions.
Copyright (C) 1995 Free Software Foundation, Inc.
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

int
fcvt_r (value, ndigit, decpt, sign, buf, len)
     double value;
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

  n = snprintf (buf, len, "%.*f", ndigit, value);
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

int
ecvt_r (value, ndigit, decpt, sign, buf, len)
     double value;
     int ndigit, *decpt, *sign;
     char *buf;
     size_t len;
{
  ndigit -= (int) floor (log10 (value < 0.0 ? -value : value));
  if (ndigit < 0)
    ndigit = 0;
  return fcvt_r (value, ndigit, decpt, sign, buf, len);
}
