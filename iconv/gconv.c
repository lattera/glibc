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

#include <assert.h>
#include <gconv.h>
#include <sys/param.h>


int
internal_function
__gconv (gconv_t cd, const char **inbuf, const char *inbufend, char **outbuf,
	 char *outbufend, size_t *converted)
{
  size_t last_step = cd->nsteps - 1;
  int result;

  if (cd == (gconv_t) -1L)
    return GCONV_ILLEGAL_DESCRIPTOR;

  assert (converted != NULL);
  *converted = 0;

  if (inbuf == NULL || *inbuf == NULL)
    /* We just flush.  */
    result = (*cd->steps->fct) (cd->steps, cd->data, NULL, NULL, converted, 1);
  else
    {
      const char *last_start;

      assert (outbuf != NULL && *outbuf != NULL);
      cd->data[last_step].outbuf = *outbuf;
      cd->data[last_step].outbufend = outbufend;

      do
	{
	  /* See whether the input size is reasoable for the output
	     size.  If not adjust it.  */
	  size_t inlen = ((inbufend - *inbuf) / cd->steps->max_needed_from
			  * cd->steps->max_needed_from);

	  if (cd->nsteps > 1)
	    inlen = MIN (inlen, (((outbufend - cd->data[last_step].outbuf)
				  / cd->steps[last_step].max_needed_to)
				 * cd->steps[last_step].max_needed_to));

	  last_start = *inbuf;
	  result = (*cd->steps->fct) (cd->steps, cd->data, inbuf,
				      *inbuf + inlen, converted, 0);
	}
      while (result == GCONV_EMPTY_INPUT && last_start != *inbuf
	     && *inbuf + cd->steps->min_needed_from <= inbufend);
    }

  if (outbuf != NULL && *outbuf != NULL)
    *outbuf = cd->data[last_step].outbuf;

  return result;
}
