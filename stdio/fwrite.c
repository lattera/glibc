/* Copyright (C) 1991, 92, 93, 94, 96, 97, 98 Free Software Foundation, Inc.
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


/* Write NMEMB chunks of SIZE bytes each from PTR onto STREAM.  */
size_t
fwrite (ptr, size, nmemb, stream)
     const void *ptr;
     size_t size;
     size_t nmemb;
     register FILE *stream;
{
  register const unsigned char *p = (const unsigned char *) ptr;
  register size_t to_write = size * nmemb;
  register size_t written = 0;
  int newlinep;
  size_t buffer_space;
  int default_func;

  if (!__validfp (stream) || !stream->__mode.__write)
    {
      __set_errno (EINVAL);
      return 0;
    }

  if (ferror (stream))
    return 0;
  if (p == NULL || to_write == 0)
    return 0;

  if (!stream->__seen || stream->__put_limit == stream->__buffer)
    {
      /* This stream has never been seen before.
	 Calling __flshfp will give it a buffer
	 and I/O functions if it needs them.  */
      if (__flshfp (stream, *p++) == EOF)
	return 0;
      if (--to_write == 0)
	return 1;
      else
	++written;
    }

  default_func
    = stream->__room_funcs.__output == __default_room_functions.__output;

  {
    int save = errno;

    if (__stdio_check_offset (stream) == EOF && errno != ESPIPE)
      {
	stream->__error = 1;
	goto done;
      }

    __set_errno (save);
  }

  if (stream->__buffer == NULL && default_func &&
      stream->__offset == stream->__target)
  write_through:
    /* This is an unbuffered stream using the standard output
       buffer-flushing function, so we just do a straight write.  */
    {
      int count = (stream->__io_funcs.__write == NULL ? to_write :
		   (*stream->__io_funcs.__write) (stream->__cookie,
						  (const char *) p,
						  to_write));
      if (count > 0)
	{
	  written += count;
	  if (stream->__offset != -1)
	    {
	      stream->__offset += count;
	      stream->__target = stream->__offset;
	    }
	  to_write -= count;
	  p += count;
	}
      else
	stream->__error = 1;
      goto done;
    }

  /* We ignore the end pointer here since we want to find out how much space
     is really in the buffer, even for a line-buffered stream.  */
  buffer_space = stream->__bufsize - (stream->__bufp - stream->__buffer);

  newlinep = (stream->__linebuf &&
	      memchr ((const void *) p, '\n', to_write) != NULL);

  if (newlinep && stream->__bufp == stream->__buffer &&
      stream->__offset == stream->__target)
    /* The buffer's empty, and we want to write our data
       out soon anyway, so just write it straight out.  */
    goto write_through;

  if (stream->__bufsize == 0 && !default_func)
    {
      /* No buffer, and a special function.
	 We can't do much better than putc.  */
      while (to_write-- > 0)
	{
	  if (__flshfp (stream, *p++) == EOF)
	    break;
	  else
	    ++written;
	}
    }
  else if (!default_func || buffer_space >= to_write)
    {
      /* There is enough room in the buffer for everything we want to write
	 or the user has specified his own output buffer-flushing/expanding
	 function.  */
    fill_buffer:
      while (to_write > 0)
	{
	  register size_t n = to_write;

	  if (n > buffer_space)
	    n = buffer_space;

	  buffer_space -= n;

	  written += n;
	  to_write -= n;

	  if (n < 20)
	    while (n-- > 0)
	      *stream->__bufp++ = *p++;
	  else
	    {
	      memcpy ((void *) stream->__bufp, (void *) p, n);
	      stream->__bufp += n;
	      p += n;
	    }

	  if (to_write == 0)
	    /* Done writing.  */
	    break;
	  else if (buffer_space == 0)
	    {
	      /* We have filled the buffer, so flush it.  */
	      if (fflush (stream) == EOF)
		break;

	      /* Reset our record of the space available in the buffer,
		 since we have just flushed it.  */
	    check_space:
	      buffer_space = (stream->__bufsize -
			      (stream->__bufp - stream->__buffer));
	      if (buffer_space == 0)
		{
		  /* With a custom output-room function, flushing might
		     not create any buffer space.  Try writing a single
		     character to create the space.  */
		  if (__flshfp (stream, *p++) == EOF)
		    goto done;
		  ++written;
		  --to_write;
		  goto check_space;
		}
	    }
	}

      /* We have written all the data into the buffer.  If we are
	 line-buffered and just put a newline in the buffer, flush now to
	 make sure it gets out.  */
      if (newlinep)
	fflush (stream);
    }
  else
    {
      /* It won't all fit in the buffer.  */

      if (stream->__bufp != stream->__buffer)
	{
	  /* There are characters in the buffer.  Flush them.  */
	  if (__flshfp (stream, EOF) == EOF)
	    goto done;
	}

      /* The buffer has been flushed.
	 Now either fill it or write directly.  */

      buffer_space = stream->__bufsize - (stream->__bufp - stream->__buffer);

      if (stream->__offset == stream->__target &&
	  (buffer_space < to_write || newlinep))
	/* What we have to write is bigger than the buffer,
	   or it contains a newline and we're line-buffered,
	   so write it out.  */
	goto write_through;
      else
	/* It will fit in the buffer.  */
	goto fill_buffer;
    }

 done:;
  return (size_t) written / size;
}

weak_alias (fwrite, fwrite_unlocked)
