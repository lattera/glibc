/* Return information about the filesystem on which FILE resides.
   Copyright (C) 1996-2015 Free Software Foundation, Inc.
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
#include <sys/statfs.h>
#include <stddef.h>
#include <sysdep.h>
#include <kernel-features.h>


# if __ASSUME_STATFS64 == 0
int __no_statfs64 attribute_hidden;
#endif

/* Return information about the filesystem on which FILE resides.  */
int
__statfs64 (const char *file, struct statfs64 *buf)
{
#ifdef __NR_statfs64
# if __ASSUME_STATFS64 == 0
  if (! __no_statfs64)
# endif
    {
      int result = INLINE_SYSCALL (statfs64, 3, file, sizeof (*buf), buf);

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

  if (__statfs (file, &buf32) < 0)
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
weak_alias (__statfs64, statfs64)
