/* Mapping tables for EUC-KR handling.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jungshik Shin <jshin@pantheon.yale.edu>, 1998.

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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <ksc5601.h>

/* Direction of the transformation.  */
static int to_euckr_object;
static int from_euckr_object;


static inline void
euckr_from_ucs4(wchar_t ch, unsigned char *cp)
{
  if (ch > 0x7f)
    {
      uint16_t idx=0;

      if (ucs4_to_ksc5601 (ch, &idx))
	idx |= 0x8080;

      *cp = (unsigned char) (idx/256);
      *(cp+1) = (unsigned char) (idx & 0xff) ;
    }
  /* think about 0x5c ; '\' */
  else
    {
      *cp = (unsigned char) (0x7f & ch) ;
      *(cp+1) = (unsigned char) 0;
    }
}


int
gconv_init (struct gconv_step *step)
{
  /* Determine which direction.  */
  if (strcasestr (step->from_name, "EUC-KR") != NULL)
    step->data = &from_euckr_object;
  else if (strcasestr (step->to_name, "EUC-KR") != NULL)
    step->data = &to_euckr_object;
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

	  if (step->data == &from_euckr_object)
	    {
	      size_t inchars = *inbufsize;
	      size_t outwchars = data->outbufavail;
	      char *outbuf = data->outbuf;
	      size_t cnt = 0;

	      while (cnt < inchars
		     && (outwchars + sizeof (wchar_t) <= data->outbufsize))
		{
		  int inchar = (unsigned char) inbuf[cnt];
		  wchar_t ch;

		  /*
		    half-width Korean Currency WON sign

		    if (inchar == 0x5c)
		      ch =  0x20a9;
		    else if (inchar <= 0x7f)
		      ch = (wchar_t) inchar;
		  */

		  if (inchar <= 0x7f)
		    ch = (wchar_t) inchar;


/* 0xfe(->0x7e : row 94) and 0xc9(->0x59 : row 41) are user-defined areas */

                  else if ( inchar <= 0xa0 || inchar > 0xfe || inchar == 0xc9)
                      /* This is illegal.  */
                      ch = L'\0';
		  else
		    {
		      /* Two-byte character.  First test whether the next
			 character is also available.  */
		      int inchar2;

		      if (cnt + 1 >= inchars)
			{
			  /* The second character is not available.  Store
			     the intermediate result.  */
			  result = GCONV_INCOMPLETE_INPUT;
			  break;
			}

		      inchar2 = (unsigned char) inbuf[++cnt];

		      ch = ksc5601_to_ucs4 ((uint16_t) (inchar * 256 + inchar2)
					    & 0x7f7f);
		      if (ch == UNKNOWN_10646_CHAR)
                         ch = L'\0';

		      if (ch == L'\0')
			--cnt;
		    }

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
	      int extra = 0;

	      while (inwchars >= cnt + sizeof (wchar_t)
		     && outchars < data->outbufsize)
		{
		  wchar_t ch = *((wchar_t *) (inbuf + cnt));
		  unsigned char cp[2];

/* decomposing Hangul syllables not available in KS C 5601 into Jamos
   should be considered either here or in euckr_from_ucs4() */

                  euckr_from_ucs4(ch,cp) ;

		  if (cp[0] == '\0' && ch != 0)
		    /* Illegal character.  */
		    break;

		  outbuf[outchars] = cp[0];
		  /* Now test for a possible second byte and write this
		     if possible.  */
		  if (cp[1] != '\0')
		    {
		      if (outchars + 1 >= data->outbufsize)
			{
			  /* The result does not fit into the buffer.  */
			  extra = 1;
			  break;
			}
		      outbuf[++outchars] = cp[1];
		    }

		  ++do_write;
		  ++outchars;
		  cnt += sizeof (wchar_t);
		}
	      *inbufsize -= cnt;
	      inbuf += cnt;
	      data->outbufavail = outchars;

	      if (outchars + extra < data->outbufsize)
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
	      result = (*inbufsize > (step->data == &from_euckr_object
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
