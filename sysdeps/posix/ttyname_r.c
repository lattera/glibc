/* Copyright (C) 1991, 1992, 1993, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

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
ttyname_r (fd, buf, buflen)
     int fd;
     char *buf;
     int buflen;
{
  static const char dev[] = "/dev";
  struct stat st;
  dev_t mydev;
  ino_t myino;
  DIR *dirstream;
  struct dirent *d;
  int save = errno;

  /* Test for the absolute minimal size.  This makes life easier inside
     the loop.  */
  if (buflen < sizeof (dev) + 2)
    {
      errno = EINVAL;
      return -1;
    }

  if (fstat (fd, &st) < 0)
    return -1;
  mydev = st.st_dev;
  myino = st.st_ino;

  dirstream = opendir (dev);
  if (dirstream == NULL)
    return -1;

  /* Prepare the result buffer.  */
  memcpy (buf, dev, sizeof (dev));
  buf[sizeof (dev)] = '/';
  buflen -= sizeof (dev) + 1;

  while ((d = readdir (dirstream)) != NULL)
    if (d->d_fileno == myino)
      {
	char *cp;

	cp = stpncpy (&buf[sizeof (dev) + 1], d->d_name,
		      MIN (d->d_namlen + 1, buflen));
	cp[0] = '\0';

	if (stat (buf, &st) == 0 && st.st_dev == mydev)
	  {
	    (void) closedir (dirstream);
	    errno = save;
	    return 0;
	  }
      }

  (void) closedir (dirstream);
  errno = save;
  return -1;
}
