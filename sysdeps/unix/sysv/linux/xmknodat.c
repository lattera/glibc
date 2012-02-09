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
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include <sysdep.h>
#include <kernel-features.h>
#include <sys/syscall.h>
#include <bp-checks.h>


/* Create a device file named PATH relative to FD, with permission and
   special bits MODE and device number DEV (which can be constructed
   from major and minor device numbers with the `makedev' macro above).  */
int
__xmknodat (int vers, int fd, const char *file, mode_t mode, dev_t *dev)
{
  if (vers != _MKNOD_VER)
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* We must convert the value to dev_t type used by the kernel.  */
  unsigned long long int k_dev =  (*dev) & ((1ULL << 32) - 1);
  if (k_dev != *dev)
    {
      __set_errno (EINVAL);
      return -1;
    }

#ifdef __NR_mknodat
# ifndef __ASSUME_ATFCTS
  if (__have_atfcts >= 0)
# endif
    {
      int res = INLINE_SYSCALL (mknodat, 4, fd, file, mode,
				(unsigned int) k_dev);
# ifndef __ASSUME_ATFCTS
      if (res == -1 && errno == ENOSYS)
	__have_atfcts = -1;
      else
# endif
	return res;
    }
#endif

#ifndef __ASSUME_ATFCTS
  char *buf = NULL;

  if (fd != AT_FDCWD && file[0] != '/')
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

  return INLINE_SYSCALL (mknod, 3, CHECK_STRING (file), mode,
			 (unsigned int) k_dev);
#endif
}

libc_hidden_def (__xmknodat)
