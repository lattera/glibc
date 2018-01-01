/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
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

#include "ttyname.h"

#if 0
/* Is this used anywhere?  It is not exported.  */
char *__ttyname;
#endif

static char *getttyname (const char *dev, const struct stat64 *mytty,
			 int save, int *dostat);

libc_freeres_ptr (static char *getttyname_name);

static char *
attribute_compat_text_section
getttyname (const char *dev, const struct stat64 *mytty, int save, int *dostat)
{
  static size_t namelen;
  struct stat64 st;
  DIR *dirstream;
  struct dirent64 *d;
  size_t devlen = strlen (dev) + 1;

  dirstream = __opendir (dev);
  if (dirstream == NULL)
    {
      *dostat = -1;
      return NULL;
    }

  /* Prepare for the loop.  If we already have a buffer copy the directory
     name we look at into it.  */
  if (devlen < namelen)
    *((char *) __mempcpy (getttyname_name, dev, devlen - 1)) = '/';

  while ((d = __readdir64 (dirstream)) != NULL)
    if ((d->d_fileno == mytty->st_ino || *dostat)
	&& strcmp (d->d_name, "stdin")
	&& strcmp (d->d_name, "stdout")
	&& strcmp (d->d_name, "stderr"))
      {
	size_t dlen = _D_ALLOC_NAMLEN (d);
	if (devlen + dlen > namelen)
	  {
	    free (getttyname_name);
	    namelen = 2 * (devlen + dlen); /* Big enough.  */
	    getttyname_name = malloc (namelen);
	    if (! getttyname_name)
	      {
		*dostat = -1;
		/* Perhaps it helps to free the directory stream buffer.  */
		(void) __closedir (dirstream);
		return NULL;
	      }
	    *((char *) __mempcpy (getttyname_name, dev, devlen - 1)) = '/';
	  }
	memcpy (&getttyname_name[devlen], d->d_name, dlen);
	if (__xstat64 (_STAT_VER, getttyname_name, &st) == 0
	    && is_mytty (mytty, &st))
	  {
	    (void) __closedir (dirstream);
#if 0
	    __ttyname = getttyname_name;
#endif
	    __set_errno (save);
	    return getttyname_name;
	  }
      }

  (void) __closedir (dirstream);
  __set_errno (save);
  return NULL;
}


/* Static buffer in `ttyname'.  */
libc_freeres_ptr (static char *ttyname_buf);


/* Return the pathname of the terminal FD is open on, or NULL on errors.
   The returned storage is good only until the next call to this function.  */
char *
ttyname (int fd)
{
  static size_t buflen;
  char procname[30];
  struct stat64 st, st1;
  int dostat = 0;
  int doispty = 0;
  char *name;
  int save = errno;
  struct termios term;

  /* isatty check, tcgetattr is used because it sets the correct
     errno (EBADF resp. ENOTTY) on error.  */
  if (__glibc_unlikely (__tcgetattr (fd, &term) < 0))
    return NULL;

  if (__fxstat64 (_STAT_VER, fd, &st) < 0)
    return NULL;

  /* We try using the /proc filesystem.  */
  *_fitoa_word (fd, __stpcpy (procname, "/proc/self/fd/"), 10, 0) = '\0';

  if (buflen == 0)
    {
      buflen = 4095;
      ttyname_buf = (char *) malloc (buflen + 1);
      if (ttyname_buf == NULL)
	{
	  buflen = 0;
	  return NULL;
	}
    }

  ssize_t len = __readlink (procname, ttyname_buf, buflen);
  if (__glibc_likely (len != -1))
    {
      if ((size_t) len >= buflen)
	return NULL;

#define UNREACHABLE_LEN strlen ("(unreachable)")
      if (len > UNREACHABLE_LEN
	  && memcmp (ttyname_buf, "(unreachable)", UNREACHABLE_LEN) == 0)
	{
	  memmove (ttyname_buf, ttyname_buf + UNREACHABLE_LEN,
		   len - UNREACHABLE_LEN);
	  len -= UNREACHABLE_LEN;
	}

      /* readlink need not terminate the string.  */
      ttyname_buf[len] = '\0';

      /* Verify readlink result, fall back on iterating through devices.  */
      if (ttyname_buf[0] == '/'
	  && __xstat64 (_STAT_VER, ttyname_buf, &st1) == 0
	  && is_mytty (&st, &st1))
	return ttyname_buf;

      doispty = 1;
    }

  if (__xstat64 (_STAT_VER, "/dev/pts", &st1) == 0 && S_ISDIR (st1.st_mode))
    {
      name = getttyname ("/dev/pts", &st, save, &dostat);
    }
  else
    {
      __set_errno (save);
      name = NULL;
    }

  if (!name && dostat != -1)
    {
      name = getttyname ("/dev", &st, save, &dostat);
    }

  if (!name && dostat != -1)
    {
      dostat = 1;
      name = getttyname ("/dev", &st, save, &dostat);
    }

  if (!name && doispty && is_pty (&st))
    {
      /* We failed to figure out the TTY's name, but we can at least
         signal that we did verify that it really is a PTY slave.
         This happens when we have inherited the file descriptor from
         a different mount namespace.  */
      __set_errno (ENODEV);
      return NULL;
    }

  return name;
}
