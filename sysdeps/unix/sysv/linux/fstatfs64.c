/* Return information about the filesystem on which FD resides.
   Copyright (C) 1996-2018 Free Software Foundation, Inc.
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
#include <string.h>
#include <stddef.h>
#include <sysdep.h>
#include <kernel_stat.h>

/* Hide the prototypes for __fstatfs and fstatfs so that GCC will not
   complain about the different function signatures if they are aliased
   to  __fstat64.  If STATFS_IS_STATFS64 is not zero then the statfs and
   statfs64 structures have an identical layout but different type names.  */

#if STATFS_IS_STATFS64
# define __fstatfs __fstatfs_disable
# define fstatfs fstatfs_disable
#endif
#include <sys/statfs.h>

#include <kernel-features.h>

/* Defined in statfs64.c.  */
extern int __no_statfs64 attribute_hidden;

/* Return information about the filesystem on which FD resides.  */
int
__fstatfs64 (int fd, struct statfs64 *buf)
{
#ifdef __NR_fstatfs64
# if __ASSUME_STATFS64 == 0
  if (! __no_statfs64)
# endif
    {
      int result = INLINE_SYSCALL (fstatfs64, 3, fd, sizeof (*buf), buf);

# if __ASSUME_STATFS64 == 0
      if (result == 0 || errno != ENOSYS)
# endif
	return result;

# if __ASSUME_STATFS64 == 0
      __no_statfs64 = 1;
# endif
    }
#endif

#if __ASSUME_STATFS64 == 0
  struct statfs buf32;

  if (__fstatfs (fd, &buf32) < 0)
    return -1;

  buf->f_type = buf32.f_type;
  buf->f_bsize = buf32.f_bsize;
  buf->f_blocks = buf32.f_blocks;
  buf->f_bfree = buf32.f_bfree;
  buf->f_bavail = buf32.f_bavail;
  buf->f_files = buf32.f_files;
  buf->f_ffree = buf32.f_ffree;
  buf->f_fsid = buf32.f_fsid;
  buf->f_namelen = buf32.f_namelen;
  buf->f_frsize = buf32.f_frsize;
  memcpy (buf->f_spare, buf32.f_spare, sizeof (buf32.f_spare));

  return 0;
#endif
}
weak_alias (__fstatfs64, fstatfs64)

#undef __fstatfs
#undef fstatfs

#if STATFS_IS_STATFS64
weak_alias (__fstatfs64, __fstatfs)
weak_alias (__fstatfs64, fstatfs)
#endif
