/* Convert characters in input buffer using conversion descriptor to
   output buffer.
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

#include <gconv.h>


int
internal_function
__gconv (gconv_t cd, const char **inbuf, size_t *inbytesleft, char **outbuf,
	 size_t *outbytesleft, size_t *converted)
{
  size_t last_step = cd->nsteps - 1;
  size_t oldinbytes = *inbytesleft;
  int result;

  if (cd == (gconv_t) -1L)
    return GCONV_ILLEGAL_DESCRIPTOR;

  cd->data[last_step].outbuf = *outbuf;
  cd->data[last_step].outbufavail = 0;
  cd->data[last_step].outbufsize = *outbytesleft;

  if (converted != NULL)
    *converted = 0;

  result = (*cd->steps->fct) (cd->steps, cd->data, *inbuf, inbytesleft,
			      converted, inbuf == NULL || *inbuf == NULL);

  *inbuf += oldinbytes - *inbytesleft;
  *outbuf += cd->data[last_step].outbufavail;
  *outbytesleft -= cd->data[last_step].outbufavail;

  return result;
}
