/* Get descriptor for character set conversion.
   Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
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
#include <errno.h>
#include <iconv.h>
#include <stdlib.h>
#include <string.h>

#include <gconv_int.h>


static inline void
strip (char *wp, const char *s)
{
  int slash_count = 0;

  while (*s != '\0')
    {
      if (isalnum (*s) || *s == '_' || *s == '-' || *s == '.')
	*wp++ = toupper (*s);
      else if (*s == '/')
	{
	  if (++slash_count == 3)
	    break;
	  *wp++ = '/';
	}
      ++s;
    }

  while (slash_count++ < 2)
    *wp++ = '/';

  *wp = '\0';
}


static char *
upstr (char *str)
{
  char *cp = str;
  while ((*cp = toupper (*cp)) != '\0')
    ++cp;
  return str;
}


iconv_t
iconv_open (const char *tocode, const char *fromcode)
{
  char *tocode_conv;
  char *fromcode_conv;
  size_t tocode_len;
  size_t fromcode_len;
  gconv_t cd;
  int res;

  /* Normalize the name.  We remove all characters beside alpha-numeric,
     '_', '-', '/', and '.'.  */
  tocode_len = strlen (tocode);
  tocode_conv = alloca (tocode_len + 3);
  strip (tocode_conv, tocode);
  tocode = tocode_conv[2] == '\0' ? upstr (tocode) : tocode_conv;

  fromcode_len = strlen (fromcode);
  fromcode_conv = alloca (fromcode_len + 3);
  strip (fromcode_conv, fromcode);
  fromcode = romcode_conv[2] == '\0' ? upstr (fromcode) : fromcode_conv;

  res = __gconv_open (tocode, fromcode, &cd);

  if (res != GCONV_OK)
    {
      /* We must set the error number according to the specs.  */
      if (res == GCONV_NOCONV || res == GCONV_NODB)
	__set_errno (EINVAL);

      return (iconv_t) -1;
    }

  return (iconv_t) cd;
}
