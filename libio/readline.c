/* fgets with ERANGE error reporting and size_t buffer length.
   Copyright (C) 2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "libioP.h"

/* Return -1 and set errno to EINVAL if it is ERANGE.  */
static ssize_t
fail_no_erange (void)
{
  if (errno == ERANGE)
    __set_errno (EINVAL);
  return -1;
}

/* Slow path for reading the line.  Called with no data in the stream
   read buffer.  Write data to [BUFFER, BUFFER_END).  */
static ssize_t
readline_slow (FILE *fp, char *buffer, char *buffer_end)
{
  char *start = buffer;

  while (buffer < buffer_end)
    {
      if (__underflow (fp) == EOF)
        {
          if (_IO_ferror_unlocked (fp))
            /* If the EOF was caused by a read error, report it.  */
            return fail_no_erange ();
          *buffer = '\0';
          /* Do not include the null terminator.  */
          return buffer - start;
        }

      /* __underflow has filled the buffer.  */
      char *readptr = fp->_IO_read_ptr;
      ssize_t readlen = fp->_IO_read_end - readptr;
      /* Make sure that __underflow really has acquired some data.  */
      assert (readlen > 0);
      char *pnl = memchr (readptr, '\n', readlen);
      if (pnl != NULL)
        {
          /* We found the terminator.  */
          size_t line_length = pnl - readptr;
          if (line_length + 2 > buffer_end - buffer)
            /* Not enough room in the caller-supplied buffer.  */
            break;
          memcpy (buffer, readptr, line_length + 1);
          buffer[line_length + 1] = '\0';
          fp->_IO_read_ptr = pnl + 1;
          /* Do not include the null terminator.  */
          return buffer - start + line_length + 1;
        }

      if (readlen >= buffer_end - buffer)
        /* Not enough room in the caller-supplied buffer.  */
        break;

      /* Save and consume the stream buffer.  */
      memcpy (buffer, readptr, readlen);
      fp->_IO_read_ptr = fp->_IO_read_end;
      buffer += readlen;
    }

  /* The line does not fit into the buffer.  */
  __set_errno (ERANGE);
  return -1;
}

ssize_t
__libc_readline_unlocked (FILE *fp, char *buffer, size_t buffer_length)
{
  char *buffer_end = buffer + buffer_length;

  /* Orient the stream.  */
  if (__builtin_expect (fp->_mode, -1) == 0)
    _IO_fwide (fp, -1);

  /* Fast path: The line terminator is found in the buffer.  */
  char *readptr = fp->_IO_read_ptr;
  ssize_t readlen = fp->_IO_read_end - readptr;
  off64_t start_offset;         /* File offset before reading anything.  */
  if (readlen > 0)
    {
      char *pnl = memchr (readptr, '\n', readlen);
      if (pnl != NULL)
        {
          size_t line_length = pnl - readptr;
          /* Account for line and null terminators.  */
          if (line_length + 2 > buffer_length)
            {
              __set_errno (ERANGE);
              return -1;
            }
          memcpy (buffer, readptr, line_length + 1);
          buffer[line_length + 1] = '\0';
          /* Consume the entire line.  */
          fp->_IO_read_ptr = pnl + 1;
          return line_length + 1;
        }

      /* If the buffer does not have enough space for what is pending
         in the stream (plus a NUL terminator), the buffer is too
         small.  */
      if (readlen + 1 > buffer_length)
        {
          __set_errno (ERANGE);
          return -1;
        }

      /* End of line not found.  We need all the buffered data.  Fall
         through to the slow path.  */
      memcpy (buffer, readptr, readlen);
      buffer += readlen;
      /* The original length is invalid after this point.  Use
         buffer_end instead.  */
#pragma GCC poison buffer_length
      /* Read the old offset before updating the read pointer.  */
      start_offset = __ftello64 (fp);
      fp->_IO_read_ptr = fp->_IO_read_end;
    }
  else
    {
      readlen = 0;
      start_offset = __ftello64 (fp);
    }

  /* Slow path: Read more data from the underlying file.  We need to
     restore the file pointer if the buffer is too small.  First,
     check if the __ftello64 call above failed.  */
  if (start_offset < 0)
    return fail_no_erange ();

  ssize_t result = readline_slow (fp, buffer, buffer_end);
  if (result < 0)
    {
      if (errno == ERANGE)
        {
          /* Restore the file pointer so that the caller may read the
             same line again.  */
          if (__fseeko64 (fp, start_offset, SEEK_SET) < 0)
            return fail_no_erange ();
          __set_errno (ERANGE);
        }
      /* Do not restore the file position on other errors; it is
         likely that the __fseeko64 call would fail, too.  */
      return -1;
    }
  return readlen + result;
}
libc_hidden_def (__libc_readline_unlocked)
