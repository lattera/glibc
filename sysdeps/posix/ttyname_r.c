/* Copyright (C) 1991, 92, 93, 95, 96 Free Software Foundation, Inc.
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
#include <limits.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/* Store at most BUFLEN character of the pathname of the terminal FD is
   open on in BUF.  Return 0 on success, -1 otherwise.  */
int
__ttyname_r (fd, buf, buflen)
     int fd;
     char *buf;
     size_t buflen;
{
  static const char dev[] = "/dev";
  struct stat st;
  dev_t mydev;
  ino_t myino;
  DIR *dirstream;
  struct dirent dirbuf, *d;
  int save = errno;

  /* Test for the absolute minimal size.  This makes life easier inside
     the loop.  */
  if (buflen < (int) (sizeof (dev) + 1))
    {
      __set_errno (ERANGE);
      return ERANGE;
    }

  if (!__isatty (fd))
    {
      __set_errno (ENOTTY);
      return ENOTTY;
    }

  if (fstat (fd, &st) < 0)
    return errno;
  mydev = st.st_dev;
  myino = st.st_ino;

  dirstream = opendir (dev);
  if (dirstream == NULL)
    return errno;

  /* Prepare the result buffer.  */
  memcpy (buf, dev, sizeof (dev) - 1);
  buf[sizeof (dev) - 1] = '/';
  buflen -= sizeof (dev);

  while (__readdir_r (dirstream, &dirbuf, &d) >= 0)
    if ((ino_t) d->d_fileno == myino)
      {
	char *cp;
	size_t needed = _D_EXACT_NAMLEN (d) + 1;

	if (needed > buflen)
	  {
	    (void) closedir (dirstream);
	    __set_errno (ERANGE);
	    return ERANGE;
	  }

	cp = __stpncpy (&buf[sizeof (dev)], d->d_name, needed);
	cp[0] = '\0';

	if (stat (buf, &st) == 0 && st.st_dev == mydev)
	  {
	    (void) closedir (dirstream);
	    __set_errno (save);
	    return 0;
	  }
      }

  (void) closedir (dirstream);
  __set_errno (save);
  /* It is not clear what to return in this case.  `isatty' says FD
     refers to a TTY but no entry in /dev has this inode.  */
  return ENOTTY;
}
weak_alias (__ttyname_r, ttyname_r)
