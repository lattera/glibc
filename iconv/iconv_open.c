/* Get descriptor for character set conversion.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

#include <errno.h>
#include <iconv.h>

#include <gconv.h>


iconv_t
iconv_open (const char *tocode, const char *fromcode)
{
  gconv_t cd;
  int res;

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
