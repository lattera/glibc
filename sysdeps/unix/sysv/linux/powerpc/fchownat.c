/* Copyright (C) 2005 Free Software Foundation, Inc.
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sysdep.h>
#include <stdlib.h>

#include <kernel-features.h>

/*
  In Linux 2.1.x the chown functions have been changed.  A new function lchown
  was introduced.  The new chown now follows symlinks - the old chown and the
  new lchown do not follow symlinks.
  This file emulates chown() under the old kernels.
*/

int
fchownat (int fd, const char *file, uid_t owner, gid_t group, int flag)
{
  if (flag & ~AT_SYMLINK_NOFOLLOW)
    {
      __set_errno (EINVAL);
      return -1;
    }

  char *buf = NULL;

  if (fd != AT_FDCWD && file[0] != '/')
    {
      size_t filelen = strlen (file);
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

#if __ASSUME_LCHOWN_SYSCALL
  if (flag & AT_SYMLINK_NOFOLLOW)
    result = INTERNAL_SYSCALL (lchown, err, 3, file, owner, group);
  else
    result = INTERNAL_SYSCALL (chown, err, 3, file, owner, group);
#else
  char link[PATH_MAX + 2];
  char path[2 * PATH_MAX + 4];
  int loopct;
  size_t filelen;
  static int libc_old_chown = 0 /* -1=old linux, 1=new linux, 0=unknown */;

  if (libc_old_chown == 1)
    {
      if (flag & AT_SYMLINK_NOFOLLOW)
	result = INTERNAL_SYSCALL (lchown, err, 3, __ptrvalue (file), owner,
				   group);
      else
	result = INTERNAL_SYSCALL (chown, err, 3, __ptrvalue (file), owner,
				   group);
      goto out;
    }

# ifdef __NR_lchown
  if (flag & AT_SYMLINK_NOFOLLOW)
    {
      result = INTERNAL_SYSCALL (lchown, err, 3, __ptrvalue (file), owner,
				 group);
      goto out;
    }

  if (libc_old_chown == 0)
    {
      result = INTERNAL_SYSCALL (chown, err, 3, __ptrvalue (file), owner,
				 group);
      if (__builtin_expect (!INTERNAL_SYSCALL_ERROR_P (result, err), 1))
	return result;
      if (INTERNAL_SYSCALL_ERRNO (result, err) != ENOSYS)
	{
	  libc_old_chown = 1;
	  goto fail;
	}
      libc_old_chown = -1;
    }
# else
  if (flag & AT_SYMLINK_NOFOLLOW)
    {
      result = INTERNAL_SYSCALL (chown, err, 3, __ptrvalue (file), owner,
				 group);
      goto out;
    }
# endif

  result = __readlink (file, link, PATH_MAX + 1);
  if (result == -1)
    {
# ifdef __NR_lchown
      result = INTERNAL_SYSCALL (lchown, err, 3, __ptrvalue (file), owner,
				 group);
# else
      result = INTERNAL_SYSCALL (chown, err, 3, __ptrvalue (file), owner,
				 group);
# endif
      goto out;
    }

  filelen = strlen (file) + 1;
  if (filelen > sizeof (path))
    {
      errno = ENAMETOOLONG;
      return -1;
    }
  memcpy (path, file, filelen);

  /* 'The system has an arbitrary limit...'  In practise, we'll hit
     ENAMETOOLONG before this, usually.  */
  for (loopct = 0; loopct < 128; ++loopct)
    {
      size_t linklen;

      if (result >= PATH_MAX + 1)
	{
	  errno = ENAMETOOLONG;
	  return -1;
	}

      link[result] = 0;  /* Null-terminate string, just-in-case.  */

      linklen = strlen (link) + 1;

      if (link[0] == '/')
	memcpy (path, link, linklen);
      else
	{
	  filelen = strlen (path);

	  while (filelen > 1 && path[filelen - 1] == '/')
	    --filelen;
	  while (filelen > 0 && path[filelen - 1] != '/')
	    --filelen;
	  if (filelen + linklen > sizeof (path))
	    {
	      errno = ENAMETOOLONG;
	      return -1;
	    }
	  memcpy (path + filelen, link, linklen);
	}

      result = __readlink (path, link, PATH_MAX + 1);

      if (result == -1)
	{
# ifdef __NR_lchown
	  result = INTERNAL_SYSCALL (lchown, err, 3, path, owner, group);
# else
	  result = INTERNAL_SYSCALL (chown, err, 3, path, owner, group);
# endif
	  goto out;
	}
    }
  __set_errno (ELOOP);
  return -1;

 out:
#endif

  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (result, err), 0))
    {
#if !__ASSUME_LCHOWN_SYSCALL
    fail:
#endif
      __atfct_seterrno (INTERNAL_SYSCALL_ERRNO (result, err), fd, buf);
      result = -1;
    }

  return result;
}
