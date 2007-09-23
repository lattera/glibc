/* Get descriptor for character set conversion.
   Copyright (C) 1997,1998,1999,2000,2001,2007 Free Software Foundation, Inc.
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

#include <alloca.h>
#include <errno.h>
#include <iconv.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <gconv_int.h>
#include "gconv_charset.h"


iconv_t
iconv_open (const char *tocode, const char *fromcode)
{
  /* Normalize the name.  We remove all characters beside alpha-numeric,
     '_', '-', '/', '.', and ':'.  */
  size_t tocode_len = strlen (tocode) + 3;
  char *tocode_conv;
  bool tocode_usealloca = __libc_use_alloca (tocode_len);
  if (tocode_usealloca)
    tocode_conv = (char *) alloca (tocode_len);
  else
    {
      tocode_conv = (char *) malloc (tocode_len);
      if (tocode_conv == NULL)
	return (iconv_t) -1;
    }
  strip (tocode_conv, tocode);
  tocode = (tocode_conv[2] == '\0' && tocode[0] != '\0'
	    ? upstr (tocode_conv, tocode) : tocode_conv);

  size_t fromcode_len = strlen (fromcode) + 3;
  char *fromcode_conv;
  bool fromcode_usealloca = __libc_use_alloca (fromcode_len);
  if (fromcode_usealloca)
    fromcode_conv = (char *) alloca (fromcode_len);
  else
    {
      fromcode_conv = (char *) malloc (fromcode_len);
      if (fromcode_conv == NULL)
	{
	  if (! tocode_usealloca)
	    free (tocode_conv);
	  return (iconv_t) -1;
	}
    }
  strip (fromcode_conv, fromcode);
  fromcode = (fromcode_conv[2] == '\0' && fromcode[0] != '\0'
	      ? upstr (fromcode_conv, fromcode) : fromcode_conv);

  __gconv_t cd;
  int res = __gconv_open (tocode, fromcode, &cd, 0);

  if (! fromcode_usealloca)
    free (fromcode_conv);
  if (! tocode_usealloca)
    free (tocode_conv);

  if (__builtin_expect (res, __GCONV_OK) != __GCONV_OK)
    {
      /* We must set the error number according to the specs.  */
      if (res == __GCONV_NOCONV || res == __GCONV_NODB)
	__set_errno (EINVAL);

      cd = (iconv_t) -1;
    }

  return (iconv_t) cd;
}
