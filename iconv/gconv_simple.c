/* Simple transformations functions.
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
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <sys/param.h>


int
__gconv_transform_dummy (struct gconv_step *step, struct gconv_step_data *data,
			 const char *inbuf, size_t *inlen, size_t *written,
			 int do_flush)
{
  size_t do_write;

  /* We have no stateful encoding.  So we don't have to do anything
     special.  */
  if (do_flush)
    do_write = 0;
  else
    {
      do_write = MIN (*inlen, data->outbufsize - data->outbufavail);

      memcpy (data->outbuf, inbuf, do_write);

      *inlen -= do_write;
      data->outbufavail += do_write;
    }

  /* ### TODO Actually, this number must be devided according to the
     size of the input charset.  I.e., if the input is in UCS4 the
     number of copied bytes must be divided by 4.  */
  if (written != NULL)
    *written = do_write;

  return GCONV_OK;
}


int
__gconv_transform_init_rstate (struct gconv_step *step,
			       struct gconv_step_data *data)
{
  /* We have to provide the transformation function an correctly initialized
     object of type `mbstate_t'.  This must be dynamically allocated.  */
  data->data = calloc (1, sizeof (mbstate_t));

  return data->data == NULL ? GCONV_NOMEM : GCONV_OK;
}


void
__gconv_transform_end_rstate (struct gconv_step_data *data)
{
  if (data->data != NULL)
    free (data->data);
}


int
__gconv_transform_ucs4_utf8 (struct gconv_step *step,
			     struct gconv_step_data *data, const char *inbuf,
			     size_t *inlen, size_t *written, int do_flush)
{
  struct gconv_step *next_step = step + 1;
  struct gconv_step_data *next_data = data + 1;
  gconv_fct fct = next_step->fct;
  size_t do_write;
  int result;

  /* If the function is called with no input this means we have to reset
     to the initial state.  The possibly partly converted input is
     dropped.  */
  if (do_flush)
    {
      /* Clear the state.  */
      memset (data->data, '\0', sizeof (mbstate_t));
      do_write = 0;

      /* Call the steps down the chain if there are any.  */
      if (data->is_last)
	result = GCONV_OK;
      else
	{
	  struct gconv_step *next_step = step + 1;
	  struct gconv_step_data *next_data = data + 1;

	  result = (*fct) (next_step, next_data, NULL, 0, written, 1);

	  /* Clear output buffer.  */
	  data->outbufavail = 0;
	}
    }
  else
    {
      do_write = 0;

      do
	{
	  const char *newinbuf = inbuf;
	  size_t actually = __wcsnrtombs (&data->outbuf[data->outbufavail],
					  (const wchar_t **) &newinbuf,
					  *inlen / sizeof (wchar_t),
					  data->outbufsize - data->outbufavail,
					  (mbstate_t *) data->data);

	  /* Remember how much we converted.  */
	  do_write += newinbuf - inbuf;
	  *inlen -= (newinbuf - inbuf) * sizeof (wchar_t);

	  data->outbufavail += actually;

	  if (data->is_last)
	    {
	      /* This is the last step.  */
	      result = (*inlen < sizeof (wchar_t)
			? GCONV_EMPTY_INPUT : GCONV_FULL_OUTPUT);
	      break;
	    }

	  /* Status so far.  */
	  result = GCONV_EMPTY_INPUT;

	  if (data->outbufavail > 0)
	    {
	      /* Call the functions below in the chain.  */
	      size_t newavail = data->outbufavail;

	      result = (*fct) (next_step, next_data, data->outbuf, &newavail,
			       written, 0);

	      /* Correct the output buffer.  */
	      if (newavail != data->outbufavail && newavail > 0)
		{
		  memmove (data->outbuf,
			   &data->outbuf[data->outbufavail - newavail],
			   newavail);
		  data->outbufavail = newavail;
		}
	    }
	}
      while (*inlen > 0 && result == GCONV_EMPTY_INPUT);
    }

  if (written != NULL && data->is_last)
    *written = do_write / sizeof (wchar_t);

  return result;
}


int
__gconv_transform_utf8_ucs4 (struct gconv_step *step,
			     struct gconv_step_data *data, const char *inbuf,
			     size_t *inlen, size_t *written, int do_flush)
{
  struct gconv_step *next_step = step + 1;
  struct gconv_step_data *next_data = data + 1;
  gconv_fct fct = next_step->fct;
  size_t do_write;
  int result;

  /* If the function is called with no input this means we have to reset
     to the initial state.  The possibly partly converted input is
     dropped.  */
  if (do_flush)
    {
      /* Clear the state.  */
      memset (data->data, '\0', sizeof (mbstate_t));
      do_write = 0;

      /* Call the steps down the chain if there are any.  */
      if (data->is_last)
	result = GCONV_OK;
      else
	{
	  struct gconv_step *next_step = step + 1;
	  struct gconv_step_data *next_data = data + 1;

	  result = (*fct) (next_step, next_data, NULL, 0, written, 1);
	}
    }
  else
    {
      do_write = 0;

      do
	{
	  const char *newinbuf = inbuf;
	  size_t actually = __mbsnrtowcs ((wchar_t *) &data->outbuf[data->outbufavail],
					  &newinbuf, *inlen,
					  ((data->outbufsize
					    - data->outbufavail)
					   / sizeof (wchar_t)),
					  (mbstate_t *) data->data);

	  /* Remember how much we converted.  */
	  do_write += actually;
	  *inlen -= newinbuf - inbuf;

	  data->outbufavail += actually * sizeof (wchar_t);

	  if (data->is_last)
	    {
	      /* This is the last step.  */
	      result = (data->outbufavail + sizeof (wchar_t) > data->outbufsize
			? GCONV_FULL_OUTPUT : GCONV_EMPTY_INPUT);
	      break;
	    }

	  /* Status so far.  */
	  result = GCONV_EMPTY_INPUT;

	  if (data->outbufavail > 0)
	    {
	      /* Call the functions below in the chain.  */
	      size_t newavail = data->outbufavail;

	      result = (*fct) (next_step, next_data, data->outbuf, &newavail,
			       written, 0);

	      /* Correct the output buffer.  */
	      if (newavail != data->outbufavail && newavail > 0)
		{
		  memmove (data->outbuf,
			   &data->outbuf[data->outbufavail - newavail],
			   newavail);
		  data->outbufavail = newavail;
		}
	    }
	}
      while (*inlen > 0 && result == GCONV_EMPTY_INPUT);
    }

  if (written != NULL && data->is_last)
    *written = do_write;

  return result;
}
