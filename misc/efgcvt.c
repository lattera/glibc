/* [efg]cvt -- compatibility functions for floating point formatting
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

char *
fcvt (value, ndigit, decpt, sign)
     double value;
     int ndigit, *decpt, *sign;
{
  static char buf[100];
  int n, i;

  *sign = value < 0.0;
  if (*sign)
    value = - value;

  n = snprintf (buf, sizeof buf, "%.*f", ndigit, value);
  if (n < 0)
    return NULL;

  i = 0;
  while (i < n && isdigit (buf[i]))
    ++i;
  *decpt = i;
  do
    ++i;
  while (! isdigit (buf[i]));
  memmove (&buf[i - *decpt], buf, n - (i - *decpt));

  return buf;
}

char *
ecvt (value, ndigit, decpt, sign)
     double value;
     int ndigit, *decpt, *sign;
{
  ndigit -= (int) floor (log10 (value));
  if (ndigit < 0)
    ndigit = 0;
  return fcvt (value, ndigit, decpt, sign);
}

char *
gcvt (value, ndigit, buf)
     double value;
     int ndigit;
     char *buf;
{
  sprintf (buf, "%.*g", ndigit, value);
  return buf;
}
