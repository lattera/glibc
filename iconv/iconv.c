/* Convert characters in input buffer using conversion descriptor to
   output buffer.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

#include <gconv_int.h>

#include <assert.h>


size_t
iconv (iconv_t cd, const char **inbuf, size_t *inbytesleft, char **outbuf,
       size_t *outbytesleft)
{
  gconv_t gcd = (gconv_t) cd;
  char *outstart = outbuf ? *outbuf : NULL;
  size_t converted;
  int result;

  if (inbuf == NULL || *inbuf == NULL)
    {
      result = __gconv (gcd, NULL, NULL, outbuf, outstart + *outbytesleft,
			&converted);
    }
  else
    {
      const char *instart = *inbuf;

      result = __gconv (gcd, inbuf, *inbuf + *inbytesleft, outbuf,
			*outbuf + *outbytesleft, &converted);

      *inbytesleft -= *inbuf - instart;
    }
  if (outstart != NULL)
    *outbytesleft -= *outbuf - outstart;

  switch (result)
    {
    case GCONV_ILLEGAL_DESCRIPTOR:
      __set_errno (EBADF);
      converted = (size_t) -1L;
      break;

    case GCONV_ILLEGAL_INPUT:
      __set_errno (EILSEQ);
      converted = (size_t) -1L;
      break;

    case GCONV_FULL_OUTPUT:
      __set_errno (E2BIG);
      converted = (size_t) -1L;
      break;

    case GCONV_INCOMPLETE_INPUT:
      __set_errno (EINVAL);
      converted = (size_t) -1L;
      break;

    case GCONV_EMPTY_INPUT:
    case GCONV_OK:
      /* Nothing.  */
      break;

    default:
      assert (!"Nothing like this should happen");
    }

  return converted;
}
