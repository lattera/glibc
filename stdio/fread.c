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


#define	default_func	__default_room_functions.__input

/* Read NMEMB chunks of SIZE bytes each from STREAM into P.  */
size_t
fread (p, size, nmemb, stream)
     void *p;
     size_t size;
     size_t nmemb;
     register FILE *stream;
{
  register char *ptr = (char *) p;
  register size_t to_read = size * nmemb;
  size_t bytes = to_read;

  if (!__validfp (stream) || !stream->__mode.__read)
    {
      __set_errno (EINVAL);
      return 0;
    }
  if (feof (stream) || ferror (stream))
    return 0;
  if (p == NULL || to_read == 0)
    return 0;

  if (!stream->__seen || stream->__buffer == NULL || stream->__pushed_back)
    {
      /* This stream has never been seen before, or it has a character
	 pushed back.  Call __fillbf to deal with those cases.  Life will
	 be simpler after this call.  */
      int c = __fillbf (stream);
      if (c == EOF)
	return 0;
      *ptr++ = c;
      if (--to_read == 0)
	return 1;
    }

 read_from_buffer:;
  if (stream->__bufp < stream->__get_limit)
    {
      /* First off, empty out the buffer.  */
      register size_t copy = stream->__get_limit - stream->__bufp;
      if (copy > to_read)
	copy = to_read;
      to_read -= copy;
      if (copy > 20)
	memcpy((void *) ptr, (void *) stream->__bufp, copy);
      else
	{
	  register size_t i;
	  for (i = 0; i < copy; ++i)
	    ptr[i] = stream->__bufp[i];
	}
      stream->__bufp += copy;
      if (to_read == 0)
	return nmemb;
      ptr += copy;
    }

  /* Reading directly into the user's buffer doesn't help when
     using a user-specified input buffer filling/expanding function,
     so we don't do it in that case.  */
  if (to_read >= stream->__bufsize &&
      stream->__room_funcs.__input == default_func &&
      stream->__offset == stream->__target)
    {
      /* Read directly into the user's buffer.  */
      if (stream->__io_funcs.__read != NULL)
	while (to_read > 0)
	  {
	    register int count;
	    count = (*stream->__io_funcs.__read) (stream->__cookie,
						  ptr, to_read);
	    if (count > 0)
	      {
		to_read -= count;
		if (stream->__offset != -1)
		  {
		    stream->__offset += count;
		    stream->__target += count;
		  }
		ptr += count;
	      }
	    else if (count == 0)
	      {
		stream->__eof = 1;
		break;
	      }
	    else
	      {
		stream->__error = 1;
		break;
	      }
	  }
      else
	stream->__eof = 1;
    }
  else
    {
      int c = __fillbf (stream);
      if (c == EOF)
	return (bytes - to_read) / size;
      *ptr++ = (char) c;
      --to_read;
      if (to_read > 0)
	goto read_from_buffer;
    }

  return (bytes - to_read) / size;
}

weak_alias (fread, fread_unlocked)
