/* Copyright (C) 1995, 1997, 2000, 2003, 2006 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <utime.h>
#include <sys/time.h>
#include <sysdep.h>
#include <kernel-features.h>


/* Change the access time of FILE to TVP[0] and
   the modification time of FILE to TVP[1].  */
int
__utimes (const char *file, const struct timeval tvp[2])
{
#ifdef __NR_utimes
  int result = INLINE_SYSCALL (utimes, 2, file, tvp);
# ifndef __ASSUME_UTIMES
  if (result != -1 || errno != ENOSYS)
# endif
    return result;
#endif

  /* The utimes() syscall does not exist or is not available in the
     used kernel.  Use utime().  For this we have to convert to the
     data format utime() expects.  */
#ifndef __ASSUME_UTIMES
  struct utimbuf buf;
  struct utimbuf *times;

  if (tvp != NULL)
    {
      times = &buf;
      buf.actime = tvp[0].tv_sec + tvp[0].tv_usec / 1000000;
      buf.modtime = tvp[1].tv_sec + tvp[1].tv_usec / 1000000;
    }
  else
    times = NULL;

  return INLINE_SYSCALL (utime, 2, file, times);
#endif
}

weak_alias (__utimes, utimes)
