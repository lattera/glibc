/* Get file-specific information about a descriptor.  Linux/Alpha version.
   Copyright (C) 1991,95,96,98,99,2000,2001,2002 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <sys/statfs.h>


static long int linux_fpathconf (int fd, int name);

/* Define this first, so it can be inlined.  */
#define __fpathconf static linux_fpathconf
#include <sysdeps/unix/sysv/linux/fpathconf.c>


/* Get file-specific information about FD.  */
long int
__pathconf (int fd, int name)
{
  if (name == _PC_FILESIZEBITS)
    {
      /* Test whether this is on a ext2 or UFS filesystem which
	 support large files.  */
      struct statfs fs;

      if (__fstatfs (fd, &fs) < 0
	  || (fs.f_type != EXT2_SUPER_MAGIC
	      && fs.f_type != UFS_MAGIC
	      && fs.f_type != UFS_CIGAM))
	return 32;

      /* This filesystem supported files >2GB.  */
      return 64;
    }

  return linux_fpathconf (fd, name);
}
