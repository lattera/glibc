/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>

#include "charset.h"
#include "stringtrans.h"


/* Global variable.  */
enum encoding_method encoding_method = ENC_UCS4;


void *xmalloc (size_t __n);
void *xrealloc (void *__p, size_t __n);


#define ADDC(ch)							      \
  do									      \
    {									      \
      char *cp;								      \
      if (bufact + (encoding_method == ENC_UCS4 ? 4 : 1) >= bufmax)	      \
	{								      \
	  bufmax *= 2;							      \
	  buf = xrealloc (buf, bufmax);					      \
	}								      \
      cp = &buf[bufact];						      \
      if (encode_char (ch, &cp) < 0)					      \
	{								      \
	  free (buf);							      \
	  return NULL;							      \
	}								      \
      bufact = cp - buf;						      \
    }									      \
  while (0)


char *
translate_string (char *str, struct charset_t *charset)
{
  char *buf;
  size_t bufact = 0;
  size_t bufmax = 56;

  buf = (char *) xmalloc (bufmax);

  while (str[0] != '\0')
    {
      char *tp;
      unsigned int value;

      if (str[0] != '<')
	{
	  ADDC (*str++);
	  continue;
	}

      tp = &str[1];
      while (tp[0] != '\0' && tp[0] != '>')
	if (tp[0] == '\\')
	  if (tp[1] != '\0')
	    tp += 2;
	  else
	    ++tp;
	else
	  ++tp;

      if (tp[0] == '\0')
	{
	  free (buf);
	  return NULL;
	}

      value = charset_find_value (&charset->char_table, str + 1,
				  tp - (str + 1));
      if ((wchar_t) value == ILLEGAL_CHAR_VALUE)
	{
	  free (buf);
	  return NULL;
	}
      else
	/* Encode string using current method.  */
	ADDC (value);

      str = &tp[1];
    }

  ADDC ('\0');

  return buf;
}


int
encode_char (unsigned int value, char **cpp)
{
  switch (encoding_method)
    {
    case ENC_UCS1:
      if (value > 255)
	return -1;
      *(*cpp)++ = (char) value;
      break;

    case ENC_UCS4:
#if __BYTE_ORDER == __BIG_ENDIAN
      *(*cpp)++ = (char) (value >> 24);
      *(*cpp)++ = (char) ((value >> 16) & 0xff);
      *(*cpp)++ = (char) ((value >> 8) & 0xff);
      *(*cpp)++ = (char) (value & 0xff);
#else
      *(*cpp)++ = (char) (value & 0xff);
      *(*cpp)++ = (char) ((value >>= 8) & 0xff);
      *(*cpp)++ = (char) ((value >>= 8) & 0xff);
      *(*cpp)++ = (char) ((value >>= 8) & 0xff);
#endif
      break;

    default:
      return -1;
    }

  return 0;
}
