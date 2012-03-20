/* Query filename corresponding to an open FD.  Linux version.
   Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <_itoa.h>

static inline const char *
fd_to_filename (int fd)
{
  char *ret = malloc (30);

  if (ret != NULL)
    {
      struct stat64 st;

      *_fitoa_word (fd, __stpcpy (ret, "/proc/self/fd/"), 10, 0) = '\0';

      /* We must make sure the file exists.  */
      if (__lxstat64 (_STAT_VER, ret, &st) < 0)
	{
	  /* /proc is not mounted or something else happened.  Don't
	     return the file name.  */
	  free (ret);
	  ret = NULL;
	}
    }
  return ret;
}
