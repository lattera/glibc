/* Conversion to and from the various ISO 646 CCS.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

/* The implementation of the conversion which can be performed by this
   module are not very sophisticated and not tuned at all.  There are
   zillions of ISO 646 derivates and supporting them all in a separate
   module is overkill since these coded character sets are hardly ever
   used anymore (except ANSI_X3.4-1968 == ASCII, which is compatible
   with ISO 8859-1).  The European variants are superceded by the
   various ISO 8859-? standards and the Asian variants are embedded in
   larger character sets.  Therefore this implementation is simply
   here to make it possible to do the conversion if it is necessary.
   The cost in the gconv-modules file is set to `2' and therefore
   allows one to easily provide a tuned implementation in case this
   proofs to be necessary.  */

#include <gconv.h>
#include <stdlib.h>
#include <string.h>

/* Direction of the transformation.  */
enum direction
{
  illegal_dir,
  to_iso646,
  from_iso646
};

enum variant
{
  illegal_var,
  US,		/* ANSI_X3.4-1968 */
  GB,		/* BS_4730 */
};

struct iso646_data
{
  enum direction dir;
  enum variant var;
};


int
gconv_init (struct gconv_step *step)
{
  /* Determine which direction.  */
  struct iso646_data *new_data;
  enum direction dir;
  enum variant var;
  int result;

  if (strcasestr (step->from_name, "ANSI_X3.4-1968") != NULL)
    {
      dir = from_iso646;
      var = US;
    }
  else if (strcasestr (step->from_name, "BS_4730") != NULL)
    {
      dir = from_iso646;
      var = GB;
    }
  else if (strcasestr (step->to_name, "ANSI_X3.4-1968") != NULL)
    {
      dir = to_iso646;
      var = US;
    }
  else if (strcasestr (step->to_name, "BS_4730") != NULL)
    {
      dir = to_iso646;
      var = GB;
    }
  else
    {
      dir = illegal_dir;
      var = illegal_var;
    }

  result = GCONV_NOCONV;
  if (dir != illegal_dir
      && ((new_data
	   = (struct iso646_data *) malloc (sizeof (struct iso646_data)))
	  != NULL))
    {
      new_data->dir = dir;
      new_data->var = var;
      step->data = new_data;
      result = GCONV_OK;
    }

  return result;
}


void
gconv_end (struct gconv_step *data)
{
  free (data->data);
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
      enum direction dir = ((struct iso646_data *) step->data)->dir;
      enum variant var = ((struct iso646_data *) step->data)->var;

      do_write = 0;

      do
	{
	  result = GCONV_OK;

	  if (dir == from_iso646)
	    {
	      size_t inchars = *inbufsize;
	      size_t outwchars = data->outbufavail;
	      char *outbuf = data->outbuf;
	      size_t cnt = 0;

	      while (cnt < inchars
		     && (outwchars + sizeof (wchar_t) <= data->outbufsize))
		{
		  switch ((unsigned char) inbuf[cnt])
		    {
		    case '\x23':
		      if (var == GB)
			*((wchar_t *) (outbuf + outwchars)) = 0xa3;
		      else
			*((wchar_t *) (outbuf + outwchars)) = 0x23;
		      break;
		    case '\x75':
		      if (var == GB)
			*((wchar_t *) (outbuf + outwchars)) = 0x203e;
		      else
			*((wchar_t *) (outbuf + outwchars)) = 0x75;
		      break;
		    default:
		      *((wchar_t *) (outbuf + outwchars)) =
			(unsigned char) inbuf[cnt];
		    case '\x80' ... '\xff':
		      /* Illegal character.  */
		      result = GCONV_ILLEGAL_INPUT;
		      goto out_from;
		    }
		  ++do_write;
		  outwchars += sizeof (wchar_t);
		  ++cnt;
		}
	    out_from:
	      *inbufsize -= cnt;
	      inbuf += cnt;
	      data->outbufavail = outwchars;
	    }
	  else
	    {
	      size_t inwchars = *inbufsize;
	      size_t outchars = data->outbufavail;
	      unsigned char *outbuf = data->outbuf;
	      size_t cnt = 0;

	      while (inwchars >= cnt + sizeof (wchar_t)
		     && outchars < data->outbufsize)
		{
		  switch (*((wchar_t *) (inbuf + cnt)))
		    {
		    case 0x23:
		      if (var == GB)
			goto out_to;
		      outbuf[outchars] = 0x23;
		      break;
		    case 0x75:
		      if (var == GB)
			goto out_to;
		      outbuf[outchars] = 0x75;
		      break;
		    case 0xa3:
		      if (var != GB)
			goto out_to;
		      outbuf[outchars] = 0x23;
		      break;
		    case 0x203e:
		      if (var != GB)
			goto out_to;
		      outbuf[outchars] = 0x75;
		      break;
		    default:
		      if (*((wchar_t *) (inbuf + cnt)) > 0x7f)
			goto out_to;
		      outbuf[outchars] =
			(unsigned char) *((wchar_t *) (inbuf + cnt));
		      break;
		    }

		  ++do_write;
		  ++outchars;
		  cnt += sizeof (wchar_t);
		}
	    out_to:
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
	      result = (*inbufsize > (dir == from_iso646
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
		memmove (data->outbuf,
			 &data->outbuf[data->outbufavail - newavail],
			 newavail);
	      data->outbufavail = newavail;
	    }
	}
      while (*inbufsize > 0 && result == GCONV_EMPTY_INPUT);
    }

  if (written != NULL && data->is_last)
    *written += do_write;

  return result;
}
