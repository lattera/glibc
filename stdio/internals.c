/* Copyright (C) 1991, 92, 93, 94, 95, 96, 97 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <string.h>


/* Make sure that FP has its functions set.  */
void
__stdio_check_funcs (register FILE *fp)
{
  if (!fp->__seen)
    {
      /* Initialize the stream's info, including buffering info.
	 This may give a buffer, change I/O functions, etc.
	 If no buffer is set (and the stream is not made explicitly
	 unbuffered), we allocate a buffer below, using the bufsize
	 set by this function.  */
      extern void __stdio_init_stream __P ((FILE *));
      fp->__room_funcs = __default_room_functions;
      fp->__io_funcs = __default_io_functions;
      __stdio_init_stream (fp);
      fp->__seen = 1;
    }
}


/* Minimum size of a buffer we will allocate by default.
   If this much memory is not available,
   the stream in question will be made unbuffered instead.  */
#define	MIN_BUFSIZE	128

/* Figure out what kind of buffering (none, line, or full)
   and what buffer size to give FP.  */
static void
init_stream (register FILE *fp)
{
  __stdio_check_funcs (fp);

  if (fp->__buffer == NULL && !fp->__userbuf)
    {
      int save;

      if (fp->__bufsize == 0)
	fp->__bufsize = BUFSIZ;

      /* Try to get however many bytes of buffering __stdio_pickbuf
	 specified, but if that much memory isn't available,
	 try half as much each time until it succeeds or the buffer
	 size becomes too small to be useful.  */
      save = errno;
      while (fp->__bufsize >= MIN_BUFSIZE)
	{
	  fp->__buffer = (char *) malloc (fp->__bufsize);
	  if (fp->__buffer == NULL)
	    fp->__bufsize /= 2;
	  else
	    break;
	}
      __set_errno (save);

      if (fp->__buffer == NULL)
	{
	  /* We can't get space for the buffer, so make it unbuffered.  */
	  fp->__userbuf = 1;
	  fp->__bufsize = 0;
	}
    }

  if (fp->__bufp == NULL)
    {
      /* Set the buffer pointer to the beginning of the buffer.  */
      fp->__bufp = fp->__buffer;
      fp->__put_limit = fp->__get_limit = fp->__buffer;
    }
}


/* Determine the current file position of STREAM if it is unknown.  */
int
__stdio_check_offset (stream)
     FILE *stream;
{
  init_stream (stream);

  if (stream->__offset == (fpos_t) -1)
    {
      /* This stream's offset is unknown or unknowable.  */
      if (stream->__io_funcs.__seek == NULL)
	{
	  /* Unknowable.  */
	  __set_errno (ESPIPE);
	  return EOF;
	}
      else
	{
	  /* Unknown.  Find it out.  */
	  fpos_t pos = (fpos_t) 0;
	  if ((*stream->__io_funcs.__seek) (stream->__cookie,
					    &pos, SEEK_CUR) < 0)
	    {
	      if (errno == ESPIPE)
		/* Object is incapable of seeking.  */
		stream->__io_funcs.__seek = NULL;
	      return EOF;
	    }
	  stream->__offset = pos;
	}
    }

  if (stream->__target == (fpos_t) -1)
    /* This stream was opened on an existing object with
       an unknown file position.  The position is now known.
       Make this the target position.  */
    stream->__target = stream->__offset;

  return 0;
}


/* Move FP's file position to its target file position,
   seeking as necessary and updating its `offset' field.
   Sets ferror(FP) (and possibly errno) for errors.  */
static void
seek_to_target (FILE *fp)
{
  int save = errno;
  if (__stdio_check_offset (fp) == EOF)
    {
      if (errno == ESPIPE)
	__set_errno (save);
      else
	fp->__error = 1;
    }
  else if (fp->__target != fp->__offset)
    {
      /* We are not at the target file position.
	 Seek to that position.  */
      if (fp->__io_funcs.__seek == NULL)
	{
	  /* We can't seek!  */
	  __set_errno (ESPIPE);
	  fp->__error = 1;
	}
      else
	{
	  fpos_t pos = fp->__target;
	  if ((*fp->__io_funcs.__seek) (fp->__cookie, &pos, SEEK_SET) < 0)
	    /* Seek failed!  */
	    fp->__error = 1;
	  else
	    {
	      fp->__offset = pos;
	      if (pos != fp->__target)
		{
		  /* Seek didn't go to the right place!
		     This should never happen.  */
#ifdef EGRATUITOUS
		  /* It happens in the Hurd when the io server doesn't
		     obey the protocol for io_seek.  */
		  __set_errno (EGRATUITOUS);
#else
		  /* I don't think this can happen in Unix.  */
		  __set_errno (ESPIPE); /* ??? */
#endif
		  fp->__error = 1;
		}
	    }
	}
    }
}

/* Flush the buffer for FP.
   If C is not EOF, it is also to be written.
   If the stream is line buffered and C is a newline, it is written
   to the output, otherwise it is put in the buffer after it has been
   flushed to avoid a system call for a single character.
   This is the default `output room' function.  */
static void
flushbuf (register FILE *fp, int c)
{
  int flush_only = c == EOF;
  size_t buffer_written;
  size_t to_write;

  /* Set if target and get_limit have already been twiddled appropriately.  */
  int twiddled = 0;

  if (fp->__put_limit == fp->__buffer)
    {
      /* The stream needs to be primed for writing.  */

      size_t buffer_offset = 0;

      if (fp->__target == -1)
	/* For an unseekable object, data recently read bears no relation
	   to data we will write later.  Discard the buffer.  */
	fp->__get_limit = fp->__buffer;
      else
	/* If the user has read some of the buffer, the target position
	   is incremented for each character he has read.  */
	fp->__target += fp->__bufp - fp->__buffer;

      if (fp->__mode.__read && fp->__room_funcs.__input != NULL &&
	  !fp->__mode.__append)
	{
	  int save = errno;
	  const int aligned = (fp->__buffer == NULL ||
			       __stdio_check_offset (fp) == EOF ||
			       fp->__target % fp->__bufsize == 0);
	  __set_errno (save);

	  if (!aligned)
	    {
	      /* Move to a block (buffer size) boundary and read in a block.
		 Then the output will be written as a whole block, too.  */
	      const size_t o = fp->__target % fp->__bufsize;
	      fp->__target -= o;
	      if ((*fp->__room_funcs.__input) (fp) == EOF && ferror (fp))
		return;
	      else
		__clearerr (fp);

	      if ((size_t) (fp->__get_limit - fp->__buffer) < o)
		/* Oops.  We didn't read enough (probably because we got EOF).
		   Forget we even mentioned it.  */
		fp->__target += o;
	      else
		/* Start bufp as far into the buffer as we were into
		   this block before we read it.  */
		buffer_offset = o;

	      /* The target position is now set to where the beginning of the
		 buffer maps to; and the get_limit was set by the input-room
		 function.  */
	      twiddled = 1;
	    }
	}

      if (fp->__buffer != NULL)
	{
	  /* Set up to write output into the buffer.  */
	  fp->__put_limit = fp->__buffer + fp->__bufsize;
	  fp->__bufp = fp->__buffer + buffer_offset;

	  if (!flush_only)
	    {
	      /* Put C in the buffer to be written out.
		 We only need to actually write it out now if
		 it is a newline on a line-buffered stream.  */
	      *fp->__bufp++ = (unsigned char) c;
	      if (!fp->__linebuf || (unsigned char) c != '\n')
		{
		  /* There is no need to flush C from the buffer right now.
		     Record that nothing was written from the buffer,
		     and go do clean-up at end.  */
		  buffer_written = 0;
		  goto end;
		}
	      else
		/* We put C in the buffer, so don't write it again later.  */
		flush_only = 1;
	    }
	}

      if ((size_t) (fp->__bufp - fp->__buffer) <= buffer_offset && flush_only)
	{
	  /* There is nothing new in the buffer, only data that
	     was read back aligned from the file.  */
	  buffer_written = 0;
	  goto end;
	}
    }

  /* If there is read data in the buffer past what was written,
     write all of that as well.  Otherwise, just write what has been
     written into the buffer.  */
  buffer_written = fp->__bufp - fp->__buffer;
  to_write = (buffer_written == 0 ? 0 :
	      fp->__get_limit > fp->__bufp ?
	      fp->__get_limit - fp->__buffer :
	      buffer_written);

  if (fp->__io_funcs.__write == NULL || (to_write == 0 && flush_only))
    {
      /* There is no writing function or we're coming from an fflush
	 call with nothing in the buffer, so just say the buffer's
	 been flushed, increment the file offset, and return.  */
      fp->__bufp = fp->__buffer;
      if (fp->__offset != -1)
	fp->__offset += to_write;
      goto end;
    }

  if (to_write > 0)
    {
      int wrote;

      /* Go to the target file position.  Don't bother if appending;
         the write will just ignore the file position anyway.  */
      if (!fp->__mode.__append)
	seek_to_target (fp);

      if (!ferror(fp))
	{
	  /* Write out the buffered data.  */
	  wrote = (*fp->__io_funcs.__write) (fp->__cookie, fp->__buffer,
					     to_write);
	  if (wrote > 0)
	    {
	      if (fp->__mode.__append)
		/* The write has written the data to the end of the file
		   and updated the file position to after the data.  Don't
		   bother to find the current position; we can get it
		   later if we need it.  */
		fp->__offset = fp->__target = -1;
	      else if (fp->__offset != -1)
		/* Record that we've moved forward in the file.  */
		fp->__offset += wrote;
	    }
	  if (wrote < (int) to_write)
	    /* The writing function should always write
	       the whole buffer unless there is an error.  */
	    fp->__error = 1;
	}
    }

  /* Reset the buffer pointer to the beginning of the buffer.  */
  fp->__bufp = fp->__buffer;

  /* If we're not just flushing, write the last character, C.  */
  if (!flush_only && !ferror (fp))
    {
      if (fp->__buffer == NULL || (fp->__linebuf && (unsigned char) c == '\n'))
	{
	  /* Either we're unbuffered, or we're line-buffered and
	     C is a newline, so really write it out immediately.  */
	  char cc = (unsigned char) c;
	  if ((*fp->__io_funcs.__write)(fp->__cookie, &cc, 1) < 1)
	    fp->__error = 1;
	  else if (fp->__offset != -1)
	    {
	      /* Record that we've moved forward in the file.  */
	      ++fp->__offset;
	      ++fp->__target;
	    }
	}
      else
	/* Just put C in the buffer.  */
	*fp->__bufp++ = (unsigned char) c;
    }

 end:

  if (!twiddled)
    {
      if (fp->__target != -1)
	/* The new target position moves up as
	   much as the user wrote into the buffer.  */
	fp->__target += buffer_written;

      /* Set the reading limit to the beginning of the buffer,
	 so the next `getc' will call __fillbf.  */
      fp->__get_limit = fp->__buffer;
    }

  if (feof (fp) || ferror (fp))
    fp->__bufp = fp->__put_limit;
}


/* Fill the buffer for FP and return the first character read (or EOF).
   This is the default `input_room' function.  */
static int
fillbuf (register FILE *fp)
{
  /* How far into the buffer we read we want to start bufp.  */
  size_t buffer_offset = 0;
  register char *buffer;
  register size_t to_read, nread = 0;
  /* This must be unsigned to avoid sign extension in return.  */
  unsigned char c;

  if (fp->__io_funcs.__read == NULL)
    {
      /* There is no read function, so always return EOF.  */
      fp->__eof = 1;
      goto end;
    }

  if (fp->__buffer == NULL)
    {
      /* We're unbuffered, so we want to read only one character.  */
      buffer = (char *) &c;
      to_read = 1;
    }
  else
    {
      /* We're buffered, so try to fill the buffer.  */
      buffer = fp->__buffer;
      to_read = fp->__bufsize;
    }

  /* We're reading, so we're not at the end-of-file.  */
  fp->__eof = 0;

  /* Go to the target file position.  */
  {
    int save = errno;
    if (__stdio_check_offset (fp) == 0 && fp->__target != fp->__offset)
      {
	/* Move to a block (buffer size) boundary.  */
	if (fp->__bufsize != 0)
	  {
	    buffer_offset = fp->__target % fp->__bufsize;
	    fp->__target -= buffer_offset;
	  }
	seek_to_target (fp);
      }
    __set_errno (save);
  }

  while (!ferror (fp) && !feof (fp) && nread <= buffer_offset)
    {
      /* Try to fill the buffer.  */
      int count = (*fp->__io_funcs.__read) (fp->__cookie, buffer, to_read);
      if (count == 0)
	fp->__eof = 1;
      else if (count < 0)
	fp->__error = 1;
      else
	{
	  buffer += count;
	  nread += count;
	  to_read -= count;
	  if (fp->__offset != -1)
	    /* Record that we've moved forward in the file.  */
	    fp->__offset += count;
	}
    }

  if (fp->__buffer == NULL)
    /* There is no buffer, so return the character we read
       without all the buffer pointer diddling.  */
    return (feof (fp) || ferror (fp)) ? EOF : c;

  /* Reset the buffer pointer to the beginning of the buffer
     (plus whatever offset we may have set above).  */
  fp->__bufp = fp->__buffer + buffer_offset;

 end:;

  if (feof (fp) || ferror (fp))
    {
      /* Set both end pointers to the beginning of the buffer so
	 the next i/o call will force a call to __fillbf/__flshfp.  */
      fp->__put_limit = fp->__get_limit = fp->__buffer;
      return EOF;
    }

  /* Set the end pointer to one past the last character we read.  */
  fp->__get_limit = fp->__buffer + nread;

  /* Make it so the next `putc' will call __flshfp.  */
  fp->__put_limit = fp->__buffer;

  /* Return the first character in the buffer.  */
  return *((unsigned char *) (fp->__bufp++));
}


/* Default I/O and room functions.  */

extern __io_read_fn __stdio_read;
extern __io_write_fn __stdio_write;
extern __io_seek_fn __stdio_seek;
extern __io_close_fn __stdio_close;
extern __io_fileno_fn __stdio_fileno;
const __io_functions __default_io_functions =
  {
    __stdio_read, __stdio_write, __stdio_seek, __stdio_close, __stdio_fileno
  };

const __room_functions __default_room_functions =
  {
    fillbuf, flushbuf
  };


/* Flush the buffer for FP and also write C if FLUSH_ONLY is nonzero.
   This is the function used by putc and fflush.  */
int
__flshfp (fp, c)
     register FILE *fp;
     int c;
{
  int flush_only = c == EOF;

  if (!__validfp (fp) || !fp->__mode.__write)
    {
      __set_errno (EINVAL);
      return EOF;
    }

  if (ferror (fp))
    return EOF;

  if (fp->__pushed_back)
    {
      /* Discard the char pushed back by ungetc.  */
      fp->__bufp = fp->__pushback_bufp;
      fp->__pushed_back = 0;
    }

  /* Make sure the stream is initialized (has functions and buffering).  */
  init_stream (fp);

  /* Do this early, so a `putc' on such a stream will never return success.  */
  if (fp->__room_funcs.__output == NULL)
    {
      /* A NULL `output room' function means
	 to always return an output error.  */
      fp->__error = 1;
      return EOF;
    }

  if (!flush_only &&
      /* Will C fit into the buffer?
	 See below about linebuf_active.  */
      fp->__bufp < (fp->__linebuf_active ? fp->__buffer + fp->__bufsize :
		    fp->__put_limit))
    {
      /* The character will fit in the buffer, so put it there.  */
      *fp->__bufp++ = (unsigned char) c;
      if (fp->__linebuf && (unsigned char) c == '\n')
	flush_only = 1;
      else
	return (unsigned char) c;
    }

  if (fp->__linebuf_active)
    /* This is an active line-buffered stream, so its put-limit is set
       to the beginning of the buffer in order to force a __flshfp call
       on each putc (see below).  We undo this hack here (by setting
       the limit to the end of the buffer) to simplify the interface
       with the output-room function.  */
    fp->__put_limit = fp->__buffer + fp->__bufsize;

  /* Make room in the buffer.  */
  (*fp->__room_funcs.__output) (fp, flush_only ? EOF : (unsigned char) c);

  if (fp->__linebuf)
    {
      /* This is a line-buffered stream, and it is now ready to do
	 some output.  We call this an "active line-buffered stream".
	 We set the put_limit to the beginning of the buffer,
	 so the next `putc' call will force a call to this function.
	 Setting the linebuf_active flag tells the code above
	 (on the next call) to undo this hackery.  */
      fp->__put_limit = fp->__buffer;
      fp->__linebuf_active = 1;
    }

  if (ferror (fp))
    return EOF;
  if (flush_only)
    return 0;
  return (unsigned char) c;
}


/* Fill the buffer for FP and return the first character read.
   This is the function used by getc.  */
int
__fillbf (fp)
     register FILE *fp;
{
  register int c;
  fpos_t new_target;

  if (!__validfp (fp) || !fp->__mode.__read)
    {
      __set_errno (EINVAL);
      return EOF;
    }

  if (fp->__pushed_back)
    {
      /* Return the char pushed back by ungetc.  */
      fp->__bufp = fp->__pushback_bufp;
      fp->__pushed_back = 0;
      return fp->__pushback;
    }

  /* Make sure the stream is initialized (has functions and buffering). */
  init_stream (fp);

  /* If we're trying to read the first character of a new
     line of input from an unbuffered or line buffered stream,
     we must flush all line-buffered output streams. */
  if (fp->__buffer == NULL || fp->__linebuf)
    {
      register FILE *f;
      for (f = __stdio_head; f != NULL; f = f->__next)
	if (__validfp (f) && f->__linebuf && f->__mode.__write)
	  (void) __flshfp (f, EOF);
    }

  /* Note we must do this after flushing all line-buffered
     streams, or else __flshfp would undo it!  */
  if (fp->__linebuf_active)
    {
      /* This is an active line-buffered stream, meaning it is in the midst
	 of writing, but has a bogus put_limit.  Restore it to normality.  */
      fp->__put_limit = fp->__buffer + fp->__bufsize;
      fp->__linebuf_active = 0;
    }

  /* We want the beginning of the buffer to now
     map to just past the last data we read.  */
  new_target = fp->__target + (fp->__get_limit - fp->__buffer);

  if (fp->__put_limit > fp->__buffer)
    {
      /* There is written data in the buffer.
	 Flush it out.  */
      if (fp->__room_funcs.__output == NULL)
	fp->__error = 1;
      else
	(*fp->__room_funcs.__output) (fp, EOF);
    }

  fp->__target = new_target;

  if (ferror (fp))
    c = EOF;
  else if (fp->__room_funcs.__input != NULL)
    {
      c = (*fp->__room_funcs.__input) (fp);
      if (fp->__buffer == NULL)
	/* This is an unbuffered stream, so the target sync above
	   won't do anything the next time around.  Instead, note that
	   we have read one character.  The (nonexistent) buffer now
	   maps to the position just past that character.  */
	++fp->__target;
    }
  else
    {
      /* A NULL `input_room' function means always return EOF.  */
      fp->__eof = 1;
      c = EOF;
    }

  return c;
}


/* Nuke a stream, but don't kill its link in the chain.  */
void
__invalidate (stream)
     register FILE *stream;
{
  /* Save its link.  */
  register FILE *next = stream->__next;

  /* Pulverize the deceased.  */
  memset((void *) stream, 0, sizeof(FILE));

  /* Restore the deceased's link.  */
  stream->__next = next;
}
