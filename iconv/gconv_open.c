/* Find matching transformation algorithms and initialize steps.
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

#include <errno.h>
#include <stdlib.h>

#include <gconv_int.h>


int
internal_function
__gconv_open (const char *toset, const char *fromset, gconv_t *handle)
{
  struct gconv_step *steps;
  size_t nsteps;
  gconv_t result = NULL;
  size_t cnt = 0;
  int res;

  res = __gconv_find_transform (toset, fromset, &steps, &nsteps);
  if (res == GCONV_OK)
    {
      /* Allocate room for handle.  */
      result = (gconv_t) malloc (sizeof (struct gconv_info)
				 + nsteps * sizeof (struct gconv_step_data));
      if (result == NULL)
	res = GCONV_NOMEM;
      else
	{
	  /* Remember the list of steps.  */
	  result->steps = steps;
	  result->nsteps = nsteps;

	  /* Clear the array for the step data.  */
	  memset (result->data, '\0',
		  nsteps * sizeof (struct gconv_step_data));

	  /* Call all initialization functions for the transformation
	     step implemenations.  */
	  for (cnt = 0; cnt < nsteps; ++cnt)
	    {
	      /* If this is the last step we must not allocate an
		 output buffer.  */
	      result->data[cnt].is_last = cnt == nsteps - 1;

	      /* Reset the counter.  */
	      result->data[cnt].invocation_counter = 0;

	      /* It's a regular use.  */
	      result->data[cnt].internal_use = 0;

	      /* We use the `mbstate_t' member in DATA.  */
	      result->data[cnt].statep = &result->data[cnt].__state;

	      /* Allocate the buffer.  */
	      if (!result->data[cnt].is_last)
		{
		  size_t size = (GCONV_NCHAR_GOAL
				 * steps[cnt].max_needed_to);

		  result->data[cnt].outbuf = (char *) malloc (size);
		  if (result->data[cnt].outbuf == NULL)
		    {
		      res = GCONV_NOMEM;
		      break;
		    }
		  result->data[cnt].outbufend = (result->data[cnt].outbuf
						 + size);
		}
	    }
	}
    }

  if (res != GCONV_OK)
    {
      /* Something went wrong.  Free all the resources.  */
      int serrno = errno;

      if (result != NULL)
	{
	  while (cnt-- > 0)
	    free (result->data[cnt].outbuf);

	  free (result);
	  result = NULL;
	}

      __gconv_close_transform (steps, nsteps);

      __set_errno (serrno);
    }

  *handle = result;
  return res;
}
