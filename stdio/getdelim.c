/* Copyright (C) 1991, 1992, 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

/* Read up to (and including) a TERMINATOR from STREAM into *LINEPTR
   (and null-terminate it). *LINEPTR is a pointer returned from malloc (or
   NULL), pointing to *N characters of space.  It is realloc'd as
   necessary.  Returns the number of characters read (not including the
   null terminator), or -1 on error or EOF.  */

ssize_t
__getdelim (lineptr, n, terminator, stream)
     char **lineptr;
     size_t *n;
     int terminator;
     FILE *stream;
{
  char *line, *p;
  size_t size, copy;

  if (!__validfp (stream) || lineptr == NULL || n == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (ferror (stream))
    return -1;

  /* Make sure we have a line buffer to start with.  */
  if (*lineptr == NULL || *n < 2) /* !seen and no buf yet need 2 chars.  */
    {
#ifndef	MAX_CANON
#define	MAX_CANON	256
#endif
      line = realloc (*lineptr, MAX_CANON);
      if (line == NULL)
	return -1;
      *lineptr = line;
      *n = MAX_CANON;
    }

  line = *lineptr;
  size = *n;

  copy = size;
  p = line;

  if (stream->__buffer == NULL && stream->__userbuf)
    {
      /* Unbuffered stream.  Not much optimization to do.  */

      while (1)
	{
	  size_t len;

	  while (--copy > 0)
	    {
	      register int c = getc (stream);
	      if (c == EOF)
		goto lose;
	      else if ((*p++ = c) == terminator)
		goto win;
	    }

	  /* Need to enlarge the line buffer.  */
	  len = p - line;
	  size *= 2;
	  line = realloc (line, size);
	  if (line == NULL)
	    goto lose;
	  *lineptr = line;
	  *n = size;
	  p = line + len;
	  copy = size - len;
	}
    }
  else
    {
      /* Leave space for the terminating null.  */
      --copy;

      if (!stream->__seen || stream->__buffer == NULL || stream->__pushed_back)
	{
	  /* Do one with getc to allocate a buffer.  */
	  int c = getc (stream);
	  if (c == EOF)
	    goto lose;
	  *p++ = c;
	  if (c == terminator)
	    goto win;
	  --copy;
	}

      while (1)
	{
	  size_t i;
	  char *found;

	  i = stream->__get_limit - stream->__bufp;
	  if (i == 0)
	    {
	      /* Refill the buffer.  */
	      int c = __fillbf (stream);
	      if (c == EOF)
		goto lose;
	      *p++ = c;
	      if (c == terminator)
		goto win;
	      --copy;
	      i = stream->__get_limit - stream->__bufp;
	    }

	  if (i > copy)
	    i = copy;

	  found = (char *) __memccpy ((void *) p, stream->__bufp, 
				      terminator, i);
	  if (found != NULL)
	    {
	      stream->__bufp += found - p;
	      p = found;
	      goto win;
	    }

	  stream->__bufp += i;
	  p += i;
	  copy -= i;
	  if (copy == 0)
	    {
	      /* Need to enlarge the line buffer.  */
	      size_t len = p - line;
	      size *= 2;
	      line = realloc (line, size);
	      if (line == NULL)
		goto lose;
	      *lineptr = line;
	      *n = size;
	      p = line + len;
	      copy = size - len;
	      /* Leave space for the terminating null.  */
	      --copy;
	    }
	}
    }

 lose:
  if (p == *lineptr)
    return -1;
  /* Return a partial line since we got an error in the middle.  */
 win:
  *p = '\0';
  return p - *lineptr;
}

weak_alias (__getdelim, getdelim)
