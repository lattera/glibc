/* Copyright (C) 1993, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU IO Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING.  If not, write to
   the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.

   As a special exception, if you link this library with files
   compiled with a GNU compiler to produce an executable, this does
   not cause the resulting executable to be covered by the GNU General
   Public License.  This exception does not however invalidate any
   other reasons why the executable file might be covered by the GNU
   General Public License.  */

#include "libioP.h"
#include <stdio.h>

char *
fgets_unlocked (buf, n, fp)
     char *buf;
     int n;
     _IO_FILE *fp;
{
  _IO_size_t count;
  char *result;
  int old_error;
  CHECK_FILE (fp, NULL);
  if (n <= 0)
    return NULL;
  /* This is very tricky since a file descriptor may be in the
     non-blocking mode. The error flag doesn't mean much in this
     case. We return an error only when there is a new error. */
  old_error = fp->_IO_file_flags & _IO_ERR_SEEN;
  fp->_IO_file_flags &= ~_IO_ERR_SEEN;
  count = _IO_getline (fp, buf, n - 1, '\n', 1);
  /* If we read in some bytes and errno is EAGAIN, that error will
     be reported for next read. */
  if (count == 0 || ((fp->_IO_file_flags & _IO_ERR_SEEN)
  		     && errno != EAGAIN))
    result = NULL;
  else
    {
      buf[count] = '\0';
      result = buf;
    }
  fp->_IO_file_flags |= old_error;
  return result;
}
