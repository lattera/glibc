/* Copyright (C) 1991, 92, 93, 96, 97, 98 Free Software Foundation, Inc.
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

#include <stdio-common/_itoa.h>

char *__ttyname = NULL;

static char * getttyname __P ((const char *dev, dev_t mydev,
			       ino_t myino, int save, int *dostat))
     internal_function;

static char *
internal_function
getttyname (dev, mydev, myino, save, dostat)
     const char *dev;
     dev_t mydev;
     ino_t myino;
     int save;
     int *dostat;
{
  static char *name;
  static size_t namelen = 0;
  struct stat st;
  DIR *dirstream;
  struct dirent *d;
  size_t devlen = strlen (dev) + 1;

  dirstream = opendir (dev);
  if (dirstream == NULL)
    {
      *dostat = -1;
      return NULL;
    }

  while ((d = readdir (dirstream)) != NULL)
    if (((ino_t) d->d_fileno == myino || *dostat)
	&& strcmp (d->d_name, "stdin")
	&& strcmp (d->d_name, "stdout")
	&& strcmp (d->d_name, "stderr"))
      {
	size_t dlen = _D_ALLOC_NAMLEN (d);
	if (devlen + dlen > namelen)
	  {
	    free (name);
	    namelen = 2 * (devlen + dlen); /* Big enough.  */
	    name = malloc (namelen);
	    if (! name)
	      {
		*dostat = -1;
		/* Perhaps it helps to free the directory stream buffer.  */
		(void) __closedir (dirstream);
		return NULL;
	      }
	    *((char *) __mempcpy (name, dev, devlen - 1)) = '/';
	  }
	memcpy (&name[devlen], d->d_name, dlen);
	if (stat (name, &st) == 0
#ifdef _STATBUF_ST_RDEV
	    && S_ISCHR (st.st_mode) && st.st_rdev == mydev
#else
	    && (ino_t) d->d_fileno == myino && st.st_dev == mydev
#endif
	   )
	  {
	    (void) __closedir (dirstream);
	    __ttyname = name;
	    __set_errno (save);
	    return name;
	  }
      }

  (void) __closedir (dirstream);
  __set_errno (save);
  return NULL;
}

/* Return the pathname of the terminal FD is open on, or NULL on errors.
   The returned storage is good only until the next call to this function.  */
char *
ttyname (fd)
     int fd;
{
  static char *buf;
  static size_t buflen = 0;
  char procname[30];
  struct stat st, st1;
  int dostat = 0;
  char *name;
  int save = errno;

  if (!__isatty (fd))
    return NULL;

  /* We try using the /proc filesystem.  */
  *_fitoa_word (fd, __stpcpy (procname, "/proc/self/fd/"), 10, 0) = '\0';

  if (buflen == 0)
    {
      buflen = 4095;
      buf = (char *) malloc (buflen + 1);
      if (buf == NULL)
	{
	  buflen = 0;
	  return NULL;
	}
    }

  if (__readlink (procname, buf, buflen) != -1
      /* This is for Linux 2.0.  */
      && buf[0] != '[')
    return buf;

  if (fstat (fd, &st) < 0)
    return NULL;

  if (stat ("/dev/pts", &st1) == 0 && S_ISDIR (st1.st_mode))
    {
#ifdef _STATBUF_ST_RDEV
      name = getttyname ("/dev/pts", st.st_rdev, st.st_ino, save, &dostat);
#else
      name = getttyname ("/dev/pts", st.st_dev, st.st_ino, save, &dostat);
#endif
    }
  else
    {
      __set_errno (save);
      name = NULL;
    }

  if (!name && dostat != -1)
    {
#ifdef _STATBUF_ST_RDEV
      name = getttyname ("/dev", st.st_rdev, st.st_ino, save, &dostat);
#else
      name = getttyname ("/dev", st.st_dev, st.st_ino, save, &dostat);
#endif
    }

  if (!name && dostat != -1)
    {
      dostat = 1;
#ifdef _STATBUF_ST_RDEV
      name = getttyname ("/dev", st.st_rdev, st.st_ino, save, &dostat);
#else
      name = getttyname ("/dev", st.st_dev, st.st_ino, save, &dostat);
#endif
    }

  return name;
}
