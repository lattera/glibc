/* Test for access to file, relative to open directory.  Linux version.
   Copyright (C) 2006, 2009 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <alloca.h>
#include <kernel-features.h>
#include <sysdep.h>


int
faccessat (fd, file, mode, flag)
     int fd;
     const char *file;
     int mode;
     int flag;
{
  if (flag & ~(AT_SYMLINK_NOFOLLOW | AT_EACCESS))
    {
      __set_errno (EINVAL);
      return -1;
    }

#ifdef __NR_faccessat
  if ((flag == 0 || ((flag & ~AT_EACCESS) == 0 && ! __libc_enable_secure))
# ifndef __ASSUME_ATFCTS
      && __have_atfcts >= 0
# endif
      )
    {
      int result = INLINE_SYSCALL (faccessat, 3, fd, file, mode);
# ifndef __ASSUME_ATFCTS
      if (result == -1 && errno == ENOSYS)
	__have_atfcts = -1;
      else
# endif
	return result;
    }
#endif

#ifndef __ASSUME_ATFCTS
  if ((!(flag & AT_EACCESS) || ! __libc_enable_secure)
# ifndef __NR_laccess		/* Linux so far has no laccess syscall.  */
      && !(flag & AT_SYMLINK_NOFOLLOW)
# endif
      )
    {
      /* If we are not set-uid or set-gid, access does the same.  */
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

      int result;
      INTERNAL_SYSCALL_DECL (err);

# ifdef __NR_laccess
      if (flag & AT_SYMLINK_NOFOLLOW)
	result = INTERNAL_SYSCALL (laccess, err, 2, file, mode);
      else
# endif
	result = INTERNAL_SYSCALL (access, err, 2, file, mode);

      if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (result, err), 0))
	{
	  __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (result, err), fd, buf);
	  result = -1;
	}

      return result;
    }
#endif

  struct stat64 stats;
  if (fstatat64 (fd, file, &stats, flag & AT_SYMLINK_NOFOLLOW))
    return -1;

  mode &= (X_OK | W_OK | R_OK);	/* Clear any bogus bits. */
#if R_OK != S_IROTH || W_OK != S_IWOTH || X_OK != S_IXOTH
# error Oops, portability assumptions incorrect.
#endif

  if (mode == F_OK)
    return 0;			/* The file exists. */

  uid_t uid = (flag & AT_EACCESS) ? __geteuid () : __getuid ();

  /* The super-user can read and write any file, and execute any file
     that anyone can execute. */
  if (uid == 0 && ((mode & X_OK) == 0
		   || (stats.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))))
    return 0;

  int granted = (uid == stats.st_uid
		 ? (unsigned int) (stats.st_mode & (mode << 6)) >> 6
		 : (stats.st_gid == ((flag & AT_EACCESS)
				     ? __getegid () : __getgid ())
		    || __group_member (stats.st_gid))
		 ? (unsigned int) (stats.st_mode & (mode << 3)) >> 3
		 : (stats.st_mode & mode));

  if (granted == mode)
    return 0;

  __set_errno (EACCES);
  return -1;
}
