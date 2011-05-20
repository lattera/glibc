/* Copyright (C) 1993,95,96,97,98,2000,2001,2002,2003,2008,2011
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
   02111-1307 USA.

   As a special exception, if you link the code in this file with
   files compiled with a GNU compiler to produce an executable,
   that does not cause the resulting executable to be covered by
   the GNU Lesser General Public License.  This exception does not
   however invalidate any other reasons why the executable file
   might be covered by the GNU Lesser General Public License.
   This exception applies to code released by its copyright holders
   in files containing the exception.  */

#include "libioP.h"
#include "stdio.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <shlib-compat.h>
#include <fd_to_filename.h>

FILE*
freopen (filename, mode, fp)
     const char* filename;
     const char* mode;
     FILE* fp;
{
  FILE *result;
  CHECK_FILE (fp, NULL);
  if (!(fp->_flags & _IO_IS_FILEBUF))
    return NULL;
  _IO_acquire_lock (fp);
  int fd = _IO_fileno (fp);
  const char *gfilename = (filename == NULL && fd >= 0
			   ? fd_to_filename (fd) : filename);
  fp->_flags2 |= _IO_FLAGS2_NOCLOSE;
#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1)
  if (&_IO_stdin_used == NULL)
    {
      /* If the shared C library is used by the application binary which
	 was linked against the older version of libio, we just use the
	 older one even for internal use to avoid trouble since a pointer
	 to the old libio may be passed into shared C library and wind
	 up here. */
      _IO_old_file_close_it (fp);
      _IO_JUMPS ((struct _IO_FILE_plus *) fp) = &_IO_old_file_jumps;
      result = _IO_old_file_fopen (fp, gfilename, mode);
    }
  else
#endif
    {
      INTUSE(_IO_file_close_it) (fp);
      _IO_JUMPS ((struct _IO_FILE_plus *) fp) = &_IO_file_jumps;
      if (_IO_vtable_offset (fp) == 0 && fp->_wide_data != NULL)
	fp->_wide_data->_wide_vtable = &_IO_wfile_jumps;
      result = INTUSE(_IO_file_fopen) (fp, gfilename, mode, 1);
      if (result != NULL)
	result = __fopen_maybe_mmap (result);
    }
  fp->_flags2 &= ~_IO_FLAGS2_NOCLOSE;
  if (result != NULL)
    {
      /* unbound stream orientation */
      result->_mode = 0;

      if (fd != -1)
	{
#ifdef O_CLOEXEC
# ifndef __ASSUME_DUP3
	  int newfd;
	  if (__have_dup3 < 0)
	    newfd = -1;
	  else
	    newfd =
# endif
	      dup3 (_IO_fileno (result), fd,
		    (result->_flags2 & _IO_FLAGS2_CLOEXEC) != 0
		    ? O_CLOEXEC : 0);
#else
# define newfd 1
#endif

#ifndef __ASSUME_DUP3
	  if (newfd < 0)
	    {
	      if (errno == ENOSYS)
		__have_dup3 = -1;

	      __dup2 (_IO_fileno (result), fd);
	      if ((result->_flags2 & _IO_FLAGS2_CLOEXEC) != 0)
		__fcntl (fd, F_SETFD, FD_CLOEXEC);
	    }
#endif
	  __close (_IO_fileno (result));
	  _IO_fileno (result) = fd;
	}
    }
  else if (fd != -1)
    __close (fd);
  if (filename == NULL)
    free ((char *) gfilename);

  _IO_release_lock (fp);
  return result;
}
