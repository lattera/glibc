/* Find matching transformation algorithms and initialize steps.
   Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
#include <string.h>

#include <gconv_int.h>


int
internal_function
__gconv_open (const char *toset, const char *fromset, __gconv_t *handle,
	      int flags)
{
  struct __gconv_step *steps;
  size_t nsteps;
  __gconv_t result = NULL;
  size_t cnt = 0;
  int res;

  res = __gconv_find_transform (toset, fromset, &steps, &nsteps, flags);
  if (res == __GCONV_OK)
    {
      /* Allocate room for handle.  */
      result = (__gconv_t) malloc (sizeof (struct __gconv_info)
				   + (nsteps
				      * sizeof (struct __gconv_step_data)));
      if (result == NULL)
	res = __GCONV_NOMEM;
      else
	{
	  /* Remember the list of steps.  */
	  result->__steps = steps;
	  result->__nsteps = nsteps;

	  /* Clear the array for the step data.  */
	  memset (result->__data, '\0',
		  nsteps * sizeof (struct __gconv_step_data));

	  /* Call all initialization functions for the transformation
	     step implementations.  */
	  for (cnt = 0; cnt < nsteps - 1; ++cnt)
	    {
	      size_t size;

	      /* If this is the last step we must not allocate an
		 output buffer.  */
	      result->__data[cnt].__is_last = 0;

	      /* Reset the counter.  */
	      result->__data[cnt].__invocation_counter = 0;

	      /* It's a regular use.  */
	      result->__data[cnt].__internal_use = 0;

	      /* We use the `mbstate_t' member in DATA.  */
	      result->__data[cnt].__statep = &result->__data[cnt].__state;

	      /* Allocate the buffer.  */
	      size = (GCONV_NCHAR_GOAL * steps[cnt].__max_needed_to);

	      result->__data[cnt].__outbuf = (char *) malloc (size);
	      if (result->__data[cnt].__outbuf == NULL)
		{
		  res = __GCONV_NOMEM;
		  break;
		}
	      result->__data[cnt].__outbufend =
		result->__data[cnt].__outbuf + size;
	    }

	  /* Now handle the last entry.  */
	  result->__data[cnt].__is_last = 1;
	  result->__data[cnt].__invocation_counter = 0;
	  result->__data[cnt].__internal_use = 0;
	  result->__data[cnt].__statep = &result->__data[cnt].__state;
	}

      if (res != __GCONV_OK)
	{
	  /* Something went wrong.  Free all the resources.  */
	  int serrno = errno;

	  if (result != NULL)
	    {
	      while (cnt-- > 0)
		free (result->__data[cnt].__outbuf);

	      free (result);
	      result = NULL;
	    }

	  __gconv_close_transform (steps, nsteps);

	  __set_errno (serrno);
	}
    }

  *handle = result;
  return res;
}
