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

#include "pty-internal.h"

/* Per the FreeBSD-3.0 manpages: pty masters are named
   /dev/pty[p-sP-S][0-9a-v].  I hope EIO is the right
   errno in the "already open" case; it doesn't say.  */
static const char pn1[] = "pqrsPQRS";
static const char pn2[] = "0123456789abcdefghijklmnopqrstuv";

/* Open the master side of a pseudoterminal and return its file
   descriptor, or -1 on error.  BSD version.  */
int
__getpt ()
{
  int fd;
  const char *i, *j;
  char namebuf[PTYNAMELEN];

  strcpy (namebuf, "/dev/pty");
  namebuf[10] = '\0';
  for (i = pn1; *i; ++i)
    {
      namebuf[8] = *i;
      for (j = pn2; *j; ++j)
        {
	  namebuf[9] = *j;
	  fd = open (namebuf, O_RDWR);
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
