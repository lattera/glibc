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
  int conv_flags = 0;
  const char *errhand;

  /* Find out whether any error handling method is specified.  */
  errhand = strchr (toset, '/');
  if (errhand != NULL)
    errhand = strchr (errhand + 1, '/');
  if (__builtin_expect (errhand != NULL, 1))
    {
      if (errhand[1] == '\0')
	errhand = NULL;
      else
	{
	  /* Make copy without the error handling description.  */
	  char *newtoset = (char *) alloca (errhand - toset + 1);

	  newtoset[errhand - toset] = '\0';
	  toset = memcpy (newtoset, toset, errhand - toset);

	  flags = __GCONV_IGNORE_ERRORS;

	  if (strcasecmp (errhand, "IGNORE") == 0)
	    {
	      /* Found it.  This means we should ignore conversion errors.  */
	      flags = __GCONV_IGNORE_ERRORS;
	      errhand = NULL;
	    }
	}
    }

  res = __gconv_find_transform (toset, fromset, &steps, &nsteps, flags);
  if (res == __GCONV_OK)
    {
      const char **csnames = NULL;
      size_t ncsnames = 0;
      __gconv_trans_fct trans_fct = NULL;
      __gconv_trans_context_fct trans_context_fct = NULL;
      __gconv_trans_init_fct trans_init_fct = NULL;
      __gconv_trans_end_fct trans_end_fct = NULL;

      if (errhand != NULL)
	{
	  /* Find the appropriate transliteration handling.  */
	  if (strcasecmp (errhand, "TRANSLIT") == 0)
	    {
	      /* It's the builtin transliteration handling.  We only
                 suport for it working on the internal encoding.  */
	      static const char *internal_trans_names[1] = { "INTERNAL" };

	      csnames = internal_trans_names;
	      ncsnames = 1;
	      trans_fct = __gconv_transliterate;
	      /* No context, init, or end function.  */
	    }
	  else if (strcasecmp (errhand, "WORK AROUND A GCC BUG") == 0)
	    {
	      trans_init_fct = (__gconv_trans_init_fct) 1;
	    }
	}

      /* Allocate room for handle.  */
      result = (__gconv_t) malloc (sizeof (struct __gconv_info)
				   + (nsteps
				      * sizeof (struct __gconv_step_data)));
      if (result == NULL)
	res = __GCONV_NOMEM;
      else
	{
	  size_t n;

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

	      /* Would have to be done if we would not clear the whole
                 array above.  */
	      /* If this is the last step we must not allocate an
		 output buffer.  */
	      result->__data[cnt].__flags = conv_flags;

#if 0
	      /* Reset the counter.  */
	      result->__data[cnt].__invocation_counter = 0;

	      /* It's a regular use.  */
	      result->__data[cnt].__internal_use = 0;
#endif

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

	      /* Now see whether we can use the transliteration module
		 for this step.  */
	      for (n = 0; n < ncsnames; ++n)
		if (strcasecmp (steps[cnt].__from_name, csnames[n]) == 0)
		  {
		    /* Match!  Now try the initializer.  */
		    if (trans_init_fct == NULL
			|| (trans_init_fct (&result->__data[cnt].__trans.__data,
					    steps[cnt].__to_name)
			    == __GCONV_OK))
		      {
			result->__data[cnt].__trans.__trans_fct = trans_fct;
			result->__data[cnt].__trans.__trans_context_fct =
			  trans_context_fct;
			result->__data[cnt].__trans.__trans_end_fct =
			  trans_end_fct;
		      }
		    break;
		  }
	    }

	  /* Now handle the last entry.  */
	  result->__data[cnt].__flags = conv_flags | __GCONV_IS_LAST;
	  /* Would have to be done if we would not clear the whole
	     array above.  */
#if 0
	  result->__data[cnt].__invocation_counter = 0;
	  result->__data[cnt].__internal_use = 0;
#endif
	  result->__data[cnt].__statep = &result->__data[cnt].__state;

	  /* Now see whether we can use the transliteration module
	     for this step.  */
	  for (n = 0; n < ncsnames; ++n)
	    if (strcasecmp (steps[cnt].__from_name, csnames[n]) == 0)
	      {
		/* Match!  Now try the initializer.  */
		if (trans_init_fct == NULL
		    || trans_init_fct (&result->__data[cnt].__trans.__data,
				       steps[cnt].__to_name)
		    == __GCONV_OK)
		  {
		    result->__data[cnt].__trans.__trans_fct = trans_fct;
		    result->__data[cnt].__trans.__trans_context_fct =
		      trans_context_fct;
		    result->__data[cnt].__trans.__trans_end_fct =
		      trans_end_fct;
		  }
		break;
	      }
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
