/* Find matching transformation algorithms and initialize steps.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <locale.h>
#include "../locale/localeinfo.h"
#include <stdlib.h>
#include <string.h>

#include <gconv_int.h>


int
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
  const char *ignore;
  bool translit = false;

  /* Find out whether any error handling method is specified.  */
  errhand = strchr (toset, '/');
  if (errhand != NULL)
    errhand = strchr (errhand + 1, '/');
  if (__glibc_likely (errhand != NULL))
    {
      if (*++errhand == '\0')
	errhand = NULL;
      else
	{
	  /* Make copy without the error handling description.  */
	  char *newtoset = (char *) alloca (errhand - toset + 1);
	  char *tok;
	  char *ptr = NULL /* Work around a bogus warning */;

	  newtoset[errhand - toset] = '\0';
	  toset = memcpy (newtoset, toset, errhand - toset);

	  /* Find the appropriate transliteration handlers.  */
	  tok = strdupa (errhand);

	  tok = __strtok_r (tok, ",", &ptr);
	  while (tok != NULL)
	    {
	      if (__strcasecmp_l (tok, "TRANSLIT", _nl_C_locobj_ptr) == 0)
		translit = true;
	      else if (__strcasecmp_l (tok, "IGNORE", _nl_C_locobj_ptr) == 0)
		/* Set the flag to ignore all errors.  */
		conv_flags |= __GCONV_IGNORE_ERRORS;

	      tok = __strtok_r (NULL, ",", &ptr);
	    }
	}
    }

  /* For the source character set we ignore the error handler specification.
     XXX Is this really always the best?  */
  ignore = strchr (fromset, '/');
  if (ignore != NULL && (ignore = strchr (ignore + 1, '/')) != NULL
      && *++ignore != '\0')
    {
      char *newfromset = (char *) alloca (ignore - fromset + 1);

      newfromset[ignore - fromset] = '\0';
      fromset = memcpy (newfromset, fromset, ignore - fromset);
    }

  /* If the string is empty define this to mean the charset of the
     currently selected locale.  */
  if (strcmp (toset, "//") == 0)
    {
      const char *codeset = _NL_CURRENT (LC_CTYPE, CODESET);
      size_t len = strlen (codeset);
      char *dest;
      toset = dest = (char *) alloca (len + 3);
      memcpy (__mempcpy (dest, codeset, len), "//", 3);
    }
  if (strcmp (fromset, "//") == 0)
    {
      const char *codeset = _NL_CURRENT (LC_CTYPE, CODESET);
      size_t len = strlen (codeset);
      char *dest;
      fromset = dest = (char *) alloca (len + 3);
      memcpy (__mempcpy (dest, codeset, len), "//", 3);
    }

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
	  for (cnt = 0; cnt < nsteps; ++cnt)
	    {
	      size_t size;

	      /* Would have to be done if we would not clear the whole
                 array above.  */
#if 0
	      /* Reset the counter.  */
	      result->__data[cnt].__invocation_counter = 0;

	      /* It's a regular use.  */
	      result->__data[cnt].__internal_use = 0;
#endif

	      /* We use the `mbstate_t' member in DATA.  */
	      result->__data[cnt].__statep = &result->__data[cnt].__state;

	      /* The builtin transliteration handling only
		 supports the internal encoding.  */
	      if (translit
		  && __strcasecmp_l (steps[cnt].__from_name,
				     "INTERNAL", _nl_C_locobj_ptr) == 0)
		conv_flags |= __GCONV_TRANSLIT;

	      /* If this is the last step we must not allocate an
		 output buffer.  */
	      if (cnt < nsteps - 1)
		{
		  result->__data[cnt].__flags = conv_flags;

		  /* Allocate the buffer.  */
		  size = (GCONV_NCHAR_GOAL * steps[cnt].__max_needed_to);

		  result->__data[cnt].__outbuf = malloc (size);
		  if (result->__data[cnt].__outbuf == NULL)
		    {
		      res = __GCONV_NOMEM;
		      goto bail;
		    }

		  result->__data[cnt].__outbufend =
		    result->__data[cnt].__outbuf + size;
		}
	      else
		{
		  /* Handle the last entry.  */
		  result->__data[cnt].__flags = conv_flags | __GCONV_IS_LAST;

		  break;
		}
	    }
	}

      if (res != __GCONV_OK)
	{
	  /* Something went wrong.  Free all the resources.  */
	  int serrno;
	bail:
	  serrno = errno;

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
