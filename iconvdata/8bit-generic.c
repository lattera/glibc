/* Generic conversion to and from 8bit charsets.
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

#include <gconv.h>
#include <string.h>

/* We use three objects to describe the operation mode.  */
static int from_8bit_object;
static int to_8bit_object;


int
gconv_init (struct gconv_step *step)
{
  /* Determine which direction.  */
  if (strcasestr (step->from_name, NAME) != NULL)
    step->data = &from_8bit_object;
  else if (strcasestr (step->to_name, NAME) != NULL)
    step->data = &to_8bit_object;
  else
    return GCONV_NOCONV;

  return GCONV_OK;
}


void
gconv_end (struct gconv_step *data)
{
  /* Nothing to do.  */
}


int
gconv (struct gconv_step *step, struct gconv_step_data *data,
       const char *inbuf, size_t *inbufsize, size_t *written, int do_flush)
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
	  result = GCONV_OK;

	  if (step->data == &from_8bit_object)
	    {
	      size_t inchars = *inbufsize;
	      size_t outwchars = data->outbufavail;
	      char *outbuf = data->outbuf;
	      size_t cnt = 0;

	      while (cnt < inchars
		     && (outwchars + sizeof (wchar_t) <= data->outbufsize))
		{
		  wchar_t ch = to_ucs4[((unsigned char *) inbuf)[cnt]];

		  if (ch == L'\0' && inbuf[cnt] != '\0')
		    {
		      /* This is an illegal character.  */
		      result = GCONV_ILLEGAL_INPUT;
		      break;
		    }

		  *((wchar_t *) (outbuf + outwchars)) = ch;
		  ++do_write;
		  outwchars += sizeof (wchar_t);
		  ++cnt;
		}
	      *inbufsize -= cnt;
	      inbuf += cnt;
	      data->outbufavail = outwchars;
	    }
	  else
	    {
	      size_t inwchars = *inbufsize;
	      size_t outchars = data->outbufavail;
	      char *outbuf = data->outbuf;
	      size_t cnt = 0;

	      while (inwchars >= cnt + sizeof (wchar_t)
		     && outchars < data->outbufsize)
		{
		  int ch = *((wchar_t *) (inbuf + cnt));

		  if (ch >= sizeof (from_ucs4) / sizeof (from_ucs4[0])
		      || ch < 0 || (from_ucs4[ch] == '\0' && ch != 0))
		    break;

		  outbuf[outchars] = from_ucs4[ch];
		  ++do_write;
		  ++outchars;
		  cnt += sizeof (wchar_t);
		}
	      *inbufsize -= cnt;
	      inbuf += cnt;
	      data->outbufavail = outchars;

	      if (outchars < data->outbufsize)
		{
		  /* If there is still room in the output buffer something
		     is wrong with the input.  */
		  if (inwchars >= cnt + sizeof (wchar_t))
		    {
		      /* An error occurred.  */
		      result = GCONV_ILLEGAL_INPUT;
		      break;
		    }
		  if (inwchars != cnt)
		    {
		      /* There are some unprocessed bytes at the end of the
			 input buffer.  */
		      result = GCONV_INCOMPLETE_INPUT;
		      break;
		    }
		}
	    }

	  if (result != GCONV_OK)
	    break;

	  if (data->is_last)
	    {
	      /* This is the last step.  */
	      result = (*inbufsize > (step->data == &from_8bit_object
				      ? 0 : sizeof (wchar_t) - 1)
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
      while (*inbufsize > 0 && result == GCONV_EMPTY_INPUT);
    }

  if (written != NULL && data->is_last)
    *written = do_write;

  return result;
}
