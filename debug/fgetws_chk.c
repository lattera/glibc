/* Copyright (C) 1993, 1995, 1996, 1997, 1998, 1999, 2001, 2005
   Free Software Foundation, Inc.
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

#include "libioP.h"
#include <wchar.h>
#include <sys/param.h>

wchar_t *
__fgetws_chk (wchar_t *buf, size_t size, int n, _IO_FILE *fp)
{
  _IO_size_t count;
  wchar_t *result;
  int old_error;
  CHECK_FILE (fp, NULL);
  if (n <= 0)
    return NULL;
  _IO_acquire_lock (fp);
  /* This is very tricky since a file descriptor may be in the
     non-blocking mode. The error flag doesn't mean much in this
     case. We return an error only when there is a new error. */
  old_error = fp->_IO_file_flags & _IO_ERR_SEEN;
  fp->_IO_file_flags &= ~_IO_ERR_SEEN;
  count = _IO_getwline (fp, buf, MIN ((size_t) n - 1, size), L'\n', 1);
  /* If we read in some bytes and errno is EAGAIN, that error will
     be reported for next read. */
  if (count == 0 || (_IO_ferror_unlocked (fp) && errno != EAGAIN))
    result = NULL;
  else if (count >= size)
    __chk_fail ();
  else
    {
      buf[count] = '\0';
      result = buf;
    }
  fp->_IO_file_flags |= old_error;
  _IO_release_lock (fp);
  return result;
}
