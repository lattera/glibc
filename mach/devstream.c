/* stdio on a Mach device port.
   Translates \n to \r\n on output, echos input.

Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <mach.h>
#include <device/device.h>
#include <errno.h>
#include <string.h>

static int
input (FILE *f)
{
  kern_return_t err;
  char *buffer;
  size_t to_read;
  mach_msg_type_number_t nread;
  char c;

  if (f->__buffer == NULL)
    {
      buffer = &c;
      to_read = 1;
    }
  else
    {
      buffer = f->__buffer;
      to_read = f->__bufsize;
    }

  f->__eof = 0;

  nread = to_read;
  err = device_read_inband ((device_t) f->__cookie, 0, f->__target,
			    to_read, buffer, &nread);

  if (err)
    {
      f->__error = 1;
      f->__bufp = f->__get_limit = f->__put_limit = f->__buffer;
      errno = err;
      return EOF;
    }

  /* Echo it back.  */
  err = device_write_inband ((device_t) f->__cookie, 0, f->__target,
			     buffer, nread, (int *) &to_read);

  if (f->__buffer == NULL)
    return (unsigned char) c;

  f->__get_limit = f->__buffer + nread;
  f->__bufp = f->__buffer;
  f->__put_limit = f->__buffer + (f->__mode.__write ? f->__bufsize : 0);
  return (unsigned char) *f->__bufp++;
}


#if 0
static void
output (FILE *f, int c)
{
  inline void write_some (const char *p, size_t to_write)
    {
      kern_return_t err;
      int wrote;
      while (to_write > 0)
	{
	  if (err = device_write ((device_t) f->__cookie, 0,
				  f->__target, (char *)p, 
				  to_write, &wrote))
	    {
	      errno = err;
	      f->__error = 1;
	      break;
	    }
	  p += wrote;
	  to_write -= wrote;
	  f->__target += wrote;
	}
    }

  if (f->__buffer != NULL)
    {
      if (f->__put_limit == f->__buffer)
	{
	  /* Prime the stream for writing.  */
	  f->__put_limit = f->__buffer + f->__bufsize;
	  f->__bufp = f->__buffer;
	  if (c != EOF)
	    {
	      *f->__bufp++ = (unsigned char) c;
	      c = EOF;
	    }
	}


      /* Write out the buffer.  */

      write_some (f->__buffer, f->__bufp - f->__buffer);

      f->__bufp = f->__buffer;
    }

  if (c != EOF && !ferror (f))
    {
      if (f->__linebuf && (unsigned char) c == '\n')
	{
	  static const char nl = '\n';
	  write_some (&nl, 1);
	}
      else
	*f->__bufp++ = (unsigned char) c;
    }
}
#endif


static void
output (FILE *f, int c)
{
  void write_some (const char *p, size_t to_write)
    {
      kern_return_t err;
      int wrote;
      while (to_write > 0)
	{
	  if (err = device_write_inband ((device_t) f->__cookie, 0,
					 f->__target, p, to_write, &wrote))
	    {
	      errno = err;
	      f->__error = 1;
	      break;
	    }
	  p += wrote;
	  to_write -= wrote;
	  f->__target += wrote;
	}
    }
  void write_crlf (void)
    {
      static const char crlf[] = "\r\n";
      write_some (crlf, 2);
    }

  if (f->__buffer == NULL)
    {
      /* The stream is unbuffered.  */

      if (c == '\n')
	write_crlf ();
      else if (c != EOF)
	{
	  char cc = (unsigned char) c;
	  write_some (&cc, 1);
	}

      return;
    }

  if (f->__put_limit == f->__buffer)
    {
      /* Prime the stream for writing.  */
      f->__put_limit = f->__buffer + f->__bufsize;
      f->__bufp = f->__buffer;
      if (c != EOF)
	{
	  *f->__bufp++ = (unsigned char) c;
	  c = EOF;
	}
    }

  {
    /* Search for newlines (LFs) in the buffer.  */

    char *start = f->__buffer, *p = start;

    while (!ferror (f) && (p = memchr (p, '\n', f->__bufp - start)))
      {
	/* Found one.  Replace it with a CR and write out through that CR.  */

	*p = '\r';
	write_some (start, p + 1 - start);

	/* Change it back to an LF; the next iteration will write it out
	   first thing.  Start the next searching iteration one char later.  */

	start = p;
	*p++ = '\n';
      }

    /* Write the remainder of the buffer.  */

    if (!ferror (f))
      write_some (start, f->__bufp - start);
  }

  f->__bufp = f->__buffer;

  if (c != EOF && !ferror (f))
    {
      if (f->__linebuf && (unsigned char) c == '\n')
	write_crlf ();
      else
	*f->__bufp++ = (unsigned char) c;
    }
}

static int
dealloc_ref (void *cookie)
{
  if (mach_port_deallocate (mach_task_self (), (mach_port_t) cookie))
    {
      errno = EINVAL;
      return -1;
    }
  return 0;
}


FILE *
mach_open_devstream (mach_port_t dev, const char *mode)
{
  FILE *stream;

  if (mach_port_mod_refs (mach_task_self (), dev, MACH_PORT_RIGHT_SEND, 1))
    {
      errno = EINVAL;
      return NULL;
    }

  stream = fopencookie ((void *) dev, mode, __default_io_functions);
  if (stream == NULL)
    {
      mach_port_deallocate (mach_task_self (), dev);
      return NULL;
    }

  stream->__room_funcs.__input = input;
  stream->__room_funcs.__output = output;
  stream->__io_funcs.__close = dealloc_ref;
  stream->__io_funcs.__seek = NULL; /* Cannot seek.  */
  stream->__io_funcs.__fileno = NULL; /* No corresponding POSIX.1 fd.  */
  stream->__seen = 1;

  return stream;
}
