/* Copyright (C) 1991, 92, 95, 96, 97, 98 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <stdio.h>
#include <string.h>

/* Reads characters from STREAM into S, until either a newline character
   is read, N - 1 characters have been read, or EOF is seen.  Returns
   the newline, unlike gets.  Finishes by appending a null character and
   returning S.  If EOF is seen before any characters have been written
   to S, the function returns NULL without appending the null character.
   If there is a file error, always return NULL.  */
char *
fgets (s, n, stream)
     char *s;
     int n;
     FILE *stream;
{
  register char *p = s;

  if (!__validfp (stream) || s == NULL || n <= 0)
    {
      __set_errno (EINVAL);
      return NULL;
    }

  if (ferror (stream))
    return NULL;

  if (stream->__buffer == NULL && stream->__userbuf)
    {
      /* Unbuffered stream.  Not much optimization to do.  */
      register int c = 0;
      while (--n > 0 && (c = getc (stream)) != EOF)
	if ((*p++ = c) == '\n')
	  break;
      if (c == EOF && (p == s || ferror (stream)))
	return NULL;
      *p = '\0';
      return s;
    }

  /* Leave space for the null.  */
  --n;

  if (n > 0 &&
      (!stream->__seen || stream->__buffer == NULL || stream->__pushed_back))
    {
      /* Do one with getc to allocate a buffer.  */
      int c = getc (stream);
      if (c == EOF)
	return NULL;
      *p++ = c;
      if (c == '\n')
	{
	  *p = '\0';
	  return s;
	}
      else
	--n;
    }

  while (n > 0)
    {
      size_t i;
      char *found;

      i = stream->__get_limit - stream->__bufp;
      if (i == 0)
	{
	  /* Refill the buffer.  */
	  int c = __fillbf (stream);
	  if (c == EOF)
	    break;
	  *p++ = c;
	  --n;
	  if (c == '\n')
	    {
	      *p = '\0';
	      return s;
	    }
	  i = stream->__get_limit - stream->__bufp;
	}

      if (i > (size_t) n)
	i = n;

      found = (char *) __memccpy ((void *) p, stream->__bufp, '\n', i);

      if (found != NULL)
	{
	  stream->__bufp += found - p;
	  p = found;
	  break;
	}

      stream->__bufp += i;
      n -= i;
      p += i;
    }

  if (p == s)
    return NULL;

  *p = '\0';
  return ferror (stream) ? NULL : s;
}

weak_alias (fgets, fgets_unlocked)
