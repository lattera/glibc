/* Simple transformations functions.
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

#include <errno.h>
#include <gconv.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <sys/param.h>

#ifndef EILSEQ
# define EILSEQ EINVAL
#endif


/* These are definitions used by some of the functions for handling
   UTF-8 encoding below.  */
static const wchar_t encoding_mask[] =
{
  ~0x7ff, ~0xffff, ~0x1fffff, ~0x3ffffff
};

static const unsigned char encoding_byte[] =
{
  0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};



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


/* Convert from ISO 646-IRV to ISO 10646/UCS4.  */
int
__gconv_transform_ascii_ucs4 (struct gconv_step *step,
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
      memset (data->statep, '\0', sizeof (mbstate_t));
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
      int save_errno = errno;
      do_write = 0;

      result = GCONV_OK;
      do
	{
	  const unsigned char *newinbuf = inbuf;
	  size_t actually = 0;
	  size_t cnt = 0;

	  while (data->outbufavail + sizeof (wchar_t) <= data->outbufsize
		 && cnt < *inlen)
	    {
	      if (*newinbuf > '\x7f')
		{
		  /* This is no correct ANSI_X3.4-1968 character.  */
		  result = GCONV_ILLEGAL_INPUT;
		  break;
		}

	      /* It's an one byte sequence.  */
	      *(wchar_t *) &data->outbuf[data->outbufavail]
		= (wchar_t) *newinbuf;
	      data->outbufavail += sizeof (wchar_t);
	      ++actually;

	      ++newinbuf;
	      ++cnt;
	    }

	  /* Remember how much we converted.  */
	  do_write += cnt * sizeof (wchar_t);
	  *inlen -= cnt;

	  /* Check whether an illegal character appeared.  */
	  if (result != GCONV_OK)
	    break;

	  if (data->is_last)
	    {
	      /* This is the last step.  */
	      result = (*inlen == 0 ? GCONV_EMPTY_INPUT : GCONV_FULL_OUTPUT);
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

      __set_errno (save_errno);
    }

  if (written != NULL && data->is_last)
    *written = do_write / sizeof (wchar_t);

  return result;
}


/* Convert from ISO 10646/UCS to ISO 646-IRV.  */
int
__gconv_transform_ucs4_ascii (struct gconv_step *step,
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
      memset (data->statep, '\0', sizeof (mbstate_t));
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
      int save_errno = errno;
      do_write = 0;

      result = GCONV_OK;
      do
	{
	  const wchar_t *newinbuf = (const wchar_t *) inbuf;
	  size_t actually = 0;
	  size_t cnt = 0;

	  while (data->outbufavail < data->outbufsize
		 && cnt + sizeof (wchar_t) <= *inlen)
	    {
	      if (*newinbuf < L'\0' || *newinbuf > L'\x7f')
		{
		  /* This is no correct ANSI_X3.4-1968 character.  */
		  result = GCONV_ILLEGAL_INPUT;
		  break;
		}

	      /* It's an one byte sequence.  */
	      data->outbuf[data->outbufavail++] = (char) *newinbuf;
	      ++actually;

	      ++newinbuf;
	      cnt += sizeof (wchar_t);
	    }

	  /* Remember how much we converted.  */
	  do_write += cnt / sizeof (wchar_t);
	  *inlen -= cnt;

	  /* Check whether an illegal character appeared.  */
	  if (result != GCONV_OK)
	    break;

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

      __set_errno (save_errno);
    }

  if (written != NULL && data->is_last)
    *written = do_write;

  return result;
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
      memset (data->statep, '\0', sizeof (mbstate_t));
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
      int save_errno = errno;
      do_write = 0;

      result = GCONV_OK;
      do
	{
	  const wchar_t *newinbuf = (const wchar_t *) inbuf;
	  size_t actually = 0;
	  size_t cnt = 0;

	  while (data->outbufavail < data->outbufsize
		 && cnt * sizeof (wchar_t) <= *inlen)
	    {
	      wchar_t wc = newinbuf[cnt];

	      if (wc < 0 && wc > 0x7fffffff)
		{
		  /* This is no correct ISO 10646 character.  */
		  result = GCONV_ILLEGAL_INPUT;
		  break;
		}

	      if (wc < 0x80)
		{
		  /* It's an one byte sequence.  */
		  data->outbuf[data->outbufavail++] = (char) wc;
		  ++actually;
		}
	      else
		{
		  size_t step;
		  size_t start;

		  for (step = 2; step < 6; ++step)
		    if ((wc & encoding_mask[step - 2]) == 0)
		      break;

		  if (data->outbufavail + step >= data->outbufsize)
		    /* Too long.  */
		    break;

		  start = data->outbufavail;
		  data->outbufavail += step;
		  actually += step;
		  data->outbuf[start] = encoding_byte[step - 2];
		  --step;
		  do
		    {
		      data->outbuf[start + step] = 0x80 | (wc & 0x3f);
		      wc >>= 6;
		    }
		  while (--step > 0);
		  data->outbuf[start] |= wc;
		}

	      ++cnt;
	    }

	  /* Remember how much we converted.  */
	  do_write += cnt * sizeof (wchar_t);
	  *inlen -= cnt * sizeof (wchar_t);

	  data->outbufavail += actually;

	  /* Check whether an illegal character appeared.  */
	  if (result != GCONV_OK)
	    break;

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

      __set_errno (save_errno);
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
      memset (data->statep, '\0', sizeof (mbstate_t));
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
      int save_errno = errno;
      do_write = 0;

      result = GCONV_OK;
      do
	{
	  wchar_t *outbuf = (wchar_t *) &data->outbuf[data->outbufavail];
	  size_t cnt = 0;
	  size_t actually = 0;

	  while (data->outbufavail + sizeof (wchar_t) <= data->outbufsize
		 && cnt < *inlen)
	    {
	      size_t start = cnt;
	      wchar_t value;
	      unsigned char byte;
	      int count;

	      /* Next input byte.  */
	      byte = inbuf[cnt++];

	      if (byte < 0x80)
		{
		  /* One byte sequence.  */
		  count = 0;
		  value = byte;
		}
	      else if ((byte & 0xe0) == 0xc0)
		{
		  count = 1;
		  value = byte & 0x1f;
		}
	      else if ((byte & 0xf0) == 0xe0)
		{
		  /* We expect three bytes.  */
		  count = 2;
		  value = byte & 0x0f;
		}
	      else if ((byte & 0xf8) == 0xf0)
		{
		  /* We expect four bytes.  */
		  count = 3;
		  value = byte & 0x07;
		}
	      else if ((byte & 0xfc) == 0xf8)
		{
		  /* We expect five bytes.  */
		  count = 4;
		  value = byte & 0x03;
		}
	      else if ((byte & 0xfe) == 0xfc)
		{
		  /* We expect six bytes.  */
		  count = 5;
		  value = byte & 0x01;
		}
	      else
		{
		  /* This is an illegal encoding.  */
		  result = GCONV_ILLEGAL_INPUT;
		  break;
		}

	      /* Read the possible remaining bytes.  */
	      while (cnt < *inbuf && count > 0)
		{
		  byte = inbuf[cnt++];
		  --count;

		  if ((byte & 0xc0) != 0x80)
		    {
		      /* This is an illegal encoding.  */
		      result = GCONV_ILLEGAL_INPUT;
		      break;
		    }

		  value <<= 6;
		  value |= byte & 0x3f;
		}

	      if (result != GCONV_OK)
		{
		  cnt = start;
		  break;
		}

	      *outbuf++ = value;
	      ++actually;
	    }

	  /* Remember how much we converted.  */
	  do_write += actually;
	  *inlen -= cnt;

	  data->outbufavail += actually * sizeof (wchar_t);

	  /* Check whether an illegal character appeared.  */
	  if (result != GCONV_OK)
	    {
	      result = GCONV_ILLEGAL_INPUT;
	      break;
	    }

	  if (*inlen == 0 && !__mbsinit (data->statep))
	    {
	      /* We have an incomplete character at the end.  */
	      result = GCONV_INCOMPLETE_INPUT;
	      break;
	    }

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

      __set_errno (save_errno);
    }

  if (written != NULL && data->is_last)
    *written = do_write;

  return result;
}


int
__gconv_transform_ucs2_ucs4 (struct gconv_step *step,
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
      memset (data->statep, '\0', sizeof (mbstate_t));
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
      int save_errno = errno;
      do_write = 0;

      do
	{
	  const uint16_t *newinbuf = (const uint16_t *) inbuf;
	  wchar_t *outbuf = (wchar_t *) &data->outbuf[data->outbufavail];
	  size_t actually = 0;

	  errno = 0;

	  while (data->outbufavail + 4 <= data->outbufsize
		 && *inlen >= 2)
	    {
	      outbuf[actually++] = *newinbuf++;
	      data->outbufavail += 4;
	      *inlen -= 2;
	    }

	  if (*inlen != 1)
	    {
	      /* We have an incomplete input character.  */
	      mbstate_t *state = data->statep;
	      state->count = 1;
	      state->value = *(uint8_t *) newinbuf;
	      --*inlen;
	    }

	  /* Remember how much we converted.  */
	  do_write += actually * sizeof (wchar_t);

	  /* Check whether an illegal character appeared.  */
	  if (errno != 0)
	    {
	      result = GCONV_ILLEGAL_INPUT;
	      break;
	    }

	  if (*inlen == 0 && !__mbsinit (data->statep))
	    {
	      /* We have an incomplete character at the end.  */
	      result = GCONV_INCOMPLETE_INPUT;
	      break;
	    }

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

      __set_errno (save_errno);
    }

  if (written != NULL && data->is_last)
    *written = do_write;

  return result;
}


int
__gconv_transform_ucs4_ucs2 (struct gconv_step *step,
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
      memset (data->statep, '\0', sizeof (mbstate_t));
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
      int save_errno = errno;
      do_write = 0;

      do
	{
	  const wchar_t *newinbuf = (const wchar_t *) inbuf;
	  uint16_t *outbuf = (uint16_t *) &data->outbuf[data->outbufavail];
	  size_t actually = 0;

	  errno = 0;

	  while (data->outbufavail + 2 <= data->outbufsize
		 && *inlen >= 4)
	    {
	      if (*newinbuf >= 0x10000)
		{
		  __set_errno (EILSEQ);
		    break;
		}
	      outbuf[actually++] = (wchar_t) *newinbuf;
	      *inlen -= 4;
	      data->outbufavail += 2;
	    }

	  if (*inlen < 4)
	    {
	      /* We have an incomplete input character.  */
	      mbstate_t *state = data->statep;
	      state->count = *inlen;
	      state->value = 0;
	      while (*inlen > 0)
		{
		  state->value <<= 8;
		  state->value += *(uint8_t *) newinbuf;
		  --*inlen;
		}
	    }

	  /* Remember how much we converted.  */
	  do_write += (const char *) newinbuf - inbuf;

	  /* Check whether an illegal character appeared.  */
	  if (errno != 0)
	    {
	      result = GCONV_ILLEGAL_INPUT;
	      break;
	    }

	  if (*inlen == 0 && !__mbsinit (data->statep))
	    {
	      /* We have an incomplete character at the end.  */
	      result = GCONV_INCOMPLETE_INPUT;
	      break;
	    }

	  if (data->is_last)
	    {
	      /* This is the last step.  */
	      result = *inlen == 0 ? GCONV_EMPTY_INPUT : GCONV_FULL_OUTPUT;
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

      __set_errno (save_errno);
    }

  if (written != NULL && data->is_last)
    *written = do_write / sizeof (wchar_t);

  return result;
}
