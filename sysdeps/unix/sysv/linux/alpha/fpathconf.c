/* Copyright (C) 1991, 1995, 1996, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <limits.h>
#include <sys/statfs.h>

#define EXT2_SUPER_MAGIC	0xef53
#define UFS_MAGIC		0x00011954
#define UFS_CIGAM		0x54190100 /* byteswapped MAGIC */

static long int default_fpathconf (int fd, int name);

/* Get file-specific information about descriptor FD.  */
long int
__fpathconf (fd, name)
     int fd;
     int name;
{
  if (fd < 0)
    {
      __set_errno (EBADF);
      return -1;
    }

  if (name == _PC_FILESIZEBITS)
    {
      /* Test whether this is on a ext2 filesystem which supports large
	 files.  */
      struct statfs fs;

      if (__fstatfs (fd, &fs) < 0
	  || (fs.f_type != EXT2_SUPER_MAGIC
	      && fs.f_type != UFS_MAGIC
	      && fs.f_type != UFS_CIGAM))
	return 32;

      /* This filesystem supported files >2GB.  */
      return 64;
    }

  /* Fallback to the generic version.  */
  return default_fpathconf (fd, name);
}

#define __fpathconf static default_fpathconf
#include <sysdeps/posix/fpathconf.c>
