/* Copyright (C) 1991, 92, 94, 95, 96, 97 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct memstream_info
  {
    char **buffer;
    size_t *bufsize;
  };

/* Enlarge STREAM's buffer.  */
static void
enlarge_buffer (register FILE *stream, int c)
{
  struct memstream_info *info = (struct memstream_info *) stream->__cookie;
  size_t need;

  if (stream->__put_limit != stream->__buffer)
    /* Record how much has actually been written into the buffer.  */
    *info->bufsize = stream->__bufp - stream->__buffer;

  if (stream->__target != -1
      && (size_t) stream->__target > *info->bufsize)
    /* Our target (where the buffer maps to) is always zero except when
       the user just did a SEEK_END fseek.  If he sought within the
       buffer, we need do nothing and will zero the target below.  If he
       sought past the end of the object, grow and zero-fill the buffer
       up to the target address.  */
    need = stream->__target;
  else
    need = *info->bufsize;

  /* We always need an extra character in the buffer.  Either we are
     writing C, or we are flushing and need to write a NUL terminator.  */
  ++need;

  if (stream->__bufsize < need)
    {
      /* Enlarge the buffer.  */
      char *newbuf;
      size_t newsize;
      if (stream->__bufsize * 2 < need)
	newsize = need;
      else
	newsize = stream->__bufsize * 2;
      newbuf = (char *) realloc ((void *) stream->__buffer, newsize);
      if (newbuf == NULL)
	{
	  stream->__error = 1;
	  return;
	}
      *info->buffer = stream->__buffer = newbuf;
      stream->__bufsize = newsize;
    }

  stream->__target = stream->__offset = 0;
  stream->__get_limit = stream->__bufp = stream->__buffer + *info->bufsize;
  stream->__put_limit = stream->__buffer + stream->__bufsize;

  need -= stream->__bufp - stream->__buffer + 1;
  if (need > 0)
    {
      /* We are extending the buffer after an fseek; zero-fill new space.  */
      memset (stream->__bufp, '\0', need);
      stream->__bufp += need;
    }

  if (c != EOF)
    *stream->__bufp++ = (unsigned char) c;
  else
    *stream->__bufp = '\0';
}

/* Seek function for memstreams.
   There is no external state to munge.  */

static int
seek (void *cookie, fpos_t *pos, int whence)
{
  switch (whence)
    {
    case SEEK_SET:
    case SEEK_CUR:
      return 0;

    case SEEK_END:
      /* Return the position relative to the end of the object.
	 fseek has just flushed us, so the info is consistent.  */
      *pos += *((struct memstream_info *) cookie)->bufsize;
      return 0;

    default:
      __libc_fatal ("memstream::seek called with bogus WHENCE\n");
      return -1;
    }
}

static int
free_info (void *cookie)
{
#if 0
  struct memstream_info *info = (struct memstream_info *) cookie;
  char *buf;

  buf = (char *) realloc ((PTR) *info->buffer, *info->bufsize);
  if (buf != NULL)
    *info->buffer = buf;
#endif

  free (cookie);

  return 0;
}

/* Open a stream that writes into a malloc'd buffer that is expanded as
   necessary.  *BUFLOC and *SIZELOC are updated with the buffer's location
   and the number of characters written on fflush or fclose.  */
FILE *
open_memstream (bufloc, sizeloc)
     char **bufloc;
     size_t *sizeloc;
{
  FILE *stream;
  struct memstream_info *info;

  if (bufloc == NULL || sizeloc == NULL)
    {
      __set_errno (EINVAL);
      return NULL;
    }

  stream = fmemopen ((char *) NULL, BUFSIZ, "w+");
  if (stream == NULL)
    return NULL;

  info = (struct memstream_info *) malloc (sizeof (struct memstream_info));
  if (info == NULL)
    {
      int save = errno;
      (void) fclose (stream);
      __set_errno (save);
      return NULL;
    }

  stream->__room_funcs.__output = enlarge_buffer;
  stream->__io_funcs.__seek = seek;
  stream->__io_funcs.__close = free_info;
  stream->__cookie = (void *) info;
  stream->__userbuf = 1;

  info->buffer = bufloc;
  info->bufsize = sizeloc;

  *bufloc = stream->__buffer;

  return stream;
}
