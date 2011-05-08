/* Copyright (C) 2005, 2006, 2011 Free Software Foundation, Inc.
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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <not-cancel.h>


DIR *
__fdopendir (int fd)
{
  struct stat64 statbuf;

  if (__builtin_expect (__fxstat64 (_STAT_VER, fd, &statbuf), 0) < 0)
    return NULL;
  if (__builtin_expect (! S_ISDIR (statbuf.st_mode), 0))
    {
      __set_errno (ENOTDIR);
      return NULL;
    }

  /* Make sure the descriptor allows for reading.  */
  int flags = __fcntl (fd, F_GETFL);
  if (__builtin_expect (flags == -1, 0))
    return NULL;
  if (__builtin_expect ((flags & O_ACCMODE) == O_WRONLY, 0))
    {
      __set_errno (EINVAL);
      return NULL;
    }

  return __alloc_dir (fd, false, flags, &statbuf);
}
weak_alias (__fdopendir, fdopendir)
