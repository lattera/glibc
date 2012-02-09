/* Copyright (C) 2005, 2006, 2009 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <utime.h>
#include <sys/time.h>
#include <sysdep.h>
#include <kernel-features.h>


/* Change the access time of FILE relative to FD to TVP[0] and
   the modification time of FILE to TVP[1].  */
int
futimesat (fd, file, tvp)
     int fd;
     const char *file;
     const struct timeval tvp[2];
{
  int result;

#ifdef __NR_futimesat
# ifndef __ASSUME_ATFCTS
  if (__have_atfcts >= 0)
# endif
    {
      if (file == NULL)
	return __futimes (fd, tvp);

      result = INLINE_SYSCALL (futimesat, 3, fd, file, tvp);
# ifndef __ASSUME_ATFCTS
      if (result == -1 && errno == ENOSYS)
	__have_atfcts = -1;
      else
# endif
	return result;
    }
#endif

#ifndef __ASSUME_ATFCTS
  char *buf = NULL;

  if (file == NULL)
    {
      static const char procfd[] = "/proc/self/fd/%d";
      /* Buffer for the path name we are going to use.  It consists of
	 - the string /proc/self/fd/
	 - the file descriptor number.
	 The final NUL is included in the sizeof.   A bit of overhead
	 due to the format elements compensates for possible negative
	 numbers.  */
      size_t buflen = sizeof (procfd) + sizeof (int) * 3;
      buf = alloca (buflen);

      __snprintf (buf, buflen, procfd, fd);
      file = buf;
    }
  else if (fd != AT_FDCWD && file[0] != '/')
    {
      size_t filelen = strlen (file);
      if (__builtin_expect (filelen == 0, 0))
	{
	  __set_errno (ENOENT);
	  return -1;
	}

      static const char procfd[] = "/proc/self/fd/%d/%s";
      /* Buffer for the path name we are going to use.  It consists of
	 - the string /proc/self/fd/
	 - the file descriptor number
	 - the file name provided.
	 The final NUL is included in the sizeof.   A bit of overhead
	 due to the format elements compensates for possible negative
	 numbers.  */
      size_t buflen = sizeof (procfd) + sizeof (int) * 3 + filelen;
      buf = alloca (buflen);

      __snprintf (buf, buflen, procfd, fd, file);
      file = buf;
    }

  INTERNAL_SYSCALL_DECL (err);

# ifdef __NR_utimes
  result = INTERNAL_SYSCALL (utimes, err, 2, file, tvp);
  if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
    return result;

#  ifndef __ASSUME_UTIMES
  if (INTERNAL_SYSCALL_ERRNO (result, err) != ENOSYS)
    goto fail;
#  endif
# endif

  /* The utimes() syscall does not exist or is not available in the
     used kernel.  Use utime().  For this we have to convert to the
     data format utime() expects.  */
# ifndef __ASSUME_UTIMES
  struct utimbuf tmp;
  struct utimbuf *times;

  if (tvp != NULL)
    {
      times = &tmp;
      tmp.actime = tvp[0].tv_sec + tvp[0].tv_usec / 1000000;
      tmp.modtime = tvp[1].tv_sec + tvp[1].tv_usec / 1000000;
    }
  else
    times = NULL;

  result = INTERNAL_SYSCALL (utime, err, 2, file, times);
  if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
    return result;

 fail:
# endif

  __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (result, err), fd, buf);

  return -1;
#endif
}
