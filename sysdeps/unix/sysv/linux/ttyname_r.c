/* Copyright (C) 1991-1993,1995-2001,2003,2006,2010
   Free Software Foundation, Inc.
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
#include <limits.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <_itoa.h>
#include <kernel-features.h>

static int getttyname_r (char *buf, size_t buflen,
			 dev_t mydev, ino64_t myino, int save,
			 int *dostat) internal_function;

static int
internal_function attribute_compat_text_section
getttyname_r (char *buf, size_t buflen, dev_t mydev, ino64_t myino,
	      int save, int *dostat)
{
  struct stat64 st;
  DIR *dirstream;
  struct dirent64 *d;
  size_t devlen = strlen (buf);

  dirstream = __opendir (buf);
  if (dirstream == NULL)
    {
      *dostat = -1;
      return errno;
    }

  while ((d = __readdir64 (dirstream)) != NULL)
    if ((d->d_fileno == myino || *dostat)
	&& strcmp (d->d_name, "stdin")
	&& strcmp (d->d_name, "stdout")
	&& strcmp (d->d_name, "stderr"))
      {
	char *cp;
	size_t needed = _D_EXACT_NAMLEN (d) + 1;

	if (needed > buflen)
	  {
	    *dostat = -1;
	    (void) __closedir (dirstream);
	    __set_errno (ERANGE);
	    return ERANGE;
	  }

	cp = __stpncpy (buf + devlen, d->d_name, needed);
	cp[0] = '\0';

	if (__xstat64 (_STAT_VER, buf, &st) == 0
#ifdef _STATBUF_ST_RDEV
	    && S_ISCHR (st.st_mode) && st.st_rdev == mydev
#else
	    && d->d_fileno == myino && st.st_dev == mydev
#endif
	   )
	  {
	    (void) __closedir (dirstream);
	    __set_errno (save);
	    return 0;
	  }
      }

  (void) __closedir (dirstream);
  __set_errno (save);
  /* It is not clear what to return in this case.  `isatty' says FD
     refers to a TTY but no entry in /dev has this inode.  */
  return ENOTTY;
}

/* Store at most BUFLEN character of the pathname of the terminal FD is
   open on in BUF.  Return 0 on success,  otherwise an error number.  */
int
__ttyname_r (int fd, char *buf, size_t buflen)
{
  char procname[30];
  struct stat64 st, st1;
  int dostat = 0;
  int save = errno;

  /* Test for the absolute minimal size.  This makes life easier inside
     the loop.  */
  if (!buf)
    {
      __set_errno (EINVAL);
      return EINVAL;
    }

  if (buflen < sizeof ("/dev/pts/"))
    {
      __set_errno (ERANGE);
      return ERANGE;
    }

  /* isatty check, tcgetattr is used because it sets the correct
     errno (EBADF resp. ENOTTY) on error.  */
  struct termios term;
  if (__builtin_expect (__tcgetattr (fd, &term) < 0, 0))
    return errno;

  if (__fxstat64 (_STAT_VER, fd, &st) < 0)
    return errno;

  /* We try using the /proc filesystem.  */
  *_fitoa_word (fd, __stpcpy (procname, "/proc/self/fd/"), 10, 0) = '\0';

  ssize_t ret = __readlink (procname, buf, buflen - 1);
  if (__builtin_expect (ret == -1 && errno == ENOENT, 0))
    {
      __set_errno (EBADF);
      return EBADF;
    }

  if (__builtin_expect (ret == -1 && errno == ENAMETOOLONG, 0))
    {
      __set_errno (ERANGE);
      return ERANGE;
    }

  if (__builtin_expect (ret != -1
#ifndef __ASSUME_PROC_SELF_FD_SYMLINK
			/* This is for Linux 2.0.  */
			&& buf[0] != '['
#endif
			, 1))
    {
#define UNREACHABLE_LEN strlen ("(unreachable)")
      if (ret > UNREACHABLE_LEN
	  && memcmp (buf, "(unreachable)", UNREACHABLE_LEN) == 0)
	{
	  memmove (buf, buf + UNREACHABLE_LEN, ret - UNREACHABLE_LEN);
	  ret -= UNREACHABLE_LEN;
	}

      /* readlink need not terminate the string.  */
      buf[ret] = '\0';

      /* Verify readlink result, fall back on iterating through devices.  */
      if (buf[0] == '/'
	  && __xstat64 (_STAT_VER, buf, &st1) == 0
#ifdef _STATBUF_ST_RDEV
	  && S_ISCHR (st1.st_mode)
	  && st1.st_rdev == st.st_rdev
#else
	  && st1.st_ino == st.st_ino
	  && st1.st_dev == st.st_dev
#endif
	  )
	return 0;
    }

  /* Prepare the result buffer.  */
  memcpy (buf, "/dev/pts/", sizeof ("/dev/pts/"));
  buflen -= sizeof ("/dev/pts/") - 1;

  if (__xstat64 (_STAT_VER, buf, &st1) == 0 && S_ISDIR (st1.st_mode))
    {
#ifdef _STATBUF_ST_RDEV
      ret = getttyname_r (buf, buflen, st.st_rdev, st.st_ino, save,
			  &dostat);
#else
      ret = getttyname_r (buf, buflen, st.st_dev, st.st_ino, save,
			  &dostat);
#endif
    }
  else
    {
      __set_errno (save);
      ret = ENOENT;
    }

  if (ret && dostat != -1)
    {
      buf[sizeof ("/dev/") - 1] = '\0';
      buflen += sizeof ("pts/") - 1;
#ifdef _STATBUF_ST_RDEV
      ret = getttyname_r (buf, buflen, st.st_rdev, st.st_ino, save,
			  &dostat);
#else
      ret = getttyname_r (buf, buflen, st.st_dev, st.st_ino, save,
			  &dostat);
#endif
    }

  if (ret && dostat != -1)
    {
      buf[sizeof ("/dev/") - 1] = '\0';
      dostat = 1;
#ifdef _STATBUF_ST_RDEV
      ret = getttyname_r (buf, buflen, st.st_rdev, st.st_ino,
			  save, &dostat);
#else
      ret = getttyname_r (buf, buflen, st.st_dev, st.st_ino,
			  save, &dostat);
#endif
    }

  return ret;
}

weak_alias (__ttyname_r, ttyname_r)
