/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Zack Weinberg <zack@rabi.phys.columbia.edu>, 1998.

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

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "pty-internal.h"

/* Per Documentation/devices.txt: pty masters are /dev/pty[p-za-e][0-9a-f].
   These strings are used also in ptsname.c. */
const char __ptyname1[] = "pqrstuvwxyzabcde";
const char __ptyname2[] = "0123456789abcdef";

/* Open the master side of a pseudoterminal and return its file
   descriptor, or -1 on error.  Linux version. */
int
__getpt ()
{
  int fd;
  const char *i, *j;
  static int have_dev_ptmx = 1;
  char namebuf[PTYNAMELEN];

  /* The new way:  */
  if (have_dev_ptmx)
    {
      fd = __open ("/dev/ptmx", O_RDWR);
      if (fd != -1)
	return fd;
      else
	{
	  if (errno == ENOENT || errno == ENODEV)
	    have_dev_ptmx = 0;
	  else
	    return -1;
	}
    }

  /* The old way: */
  strcpy (namebuf, "/dev/pty");
  namebuf[10] = '\0';
  for (i = __ptyname1; *i; ++i)
    {
      namebuf[8] = *i;
      for (j = __ptyname2; *j; ++j)
        {
	  namebuf[9] = *j;
	  fd = __open (namebuf, O_RDWR);
	  if (fd != -1)
	    return fd;
	  if (errno != EIO)
	    return -1;
        }
    }
  __set_errno (ENFILE);
  return -1;
}
weak_alias (__getpt, getpt)
