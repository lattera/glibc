/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1996.

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
#include <wchar.h>

#ifndef EILSEQ
#define EILSEQ EINVAL
#endif


/* We don't need the state really because we don't have shift states
   to maintain between calls to this function.  */
static mbstate_t internal;

size_t
__mbsrtowcs (dst, src, len, ps)
     wchar_t *dst;
     const char **src;
     size_t len;
     mbstate_t *ps;
{
  size_t written = 0;
  const char *run = *src;

  if (ps == NULL)
    ps = &internal;

  if (dst == NULL)
    /* The LEN parameter has to be ignored if we don't actually write
       anything.  */
    len = ~0;

  /* Copy all words.  */
  while (written < len)
    {
      wchar_t value;
      size_t count;
      unsigned char byte = *run++;

      /* We expect a start of a new multibyte character.  */
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
	  __set_errno (EILSEQ);
	  return (size_t) -1;
	}

      /* Read the possible remaining bytes.  */
      while (count-- > 0)
	{
	  byte = *run++;

	  if ((byte & 0xc0) != 0x80)
	    {
	      /* This is an illegal encoding.  */
	      __set_errno (EILSEQ);
	      return (size_t) -1;
	    }

	  value <<= 6;
	  value |= byte & 0x3f;
	}

      /* Store value is required.  */
      if (dst != NULL)
	*dst++ = value;

      /* The whole sequence is read.  Check whether end of string is
	 reached.  */
      if (value == L'\0')
	{
	  /* Found the end of the string.  */
	  *src = NULL;
	  return written;
	}

      /* Increment counter of produced words.  */
      ++written;
    }

  /* Store address of next byte to process.  */
  *src = run;

  return written;
}
weak_alias (__mbsrtowcs, mbsrtowcs)
