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
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "pty-internal.h"

#include <stdio-common/_itoa.h>
#include <sys/sysmacros.h>

/* Given the file descriptor of a master pty, return the pathname
   of the associated slave.  */

static char namebuf[PTYNAMELEN];
extern const char __ptyname1[], __ptyname2[]; /* Defined in getpt.c.  */

char *
ptsname (fd)
     int fd;
{
  return __ptsname_r (fd, namebuf, PTYNAMELEN);
}

char *
__ptsname_r (fd, buf, len)
     int fd;
     char *buf;
     unsigned int len;
{
  char nbuf[PTYNAMELEN], idbuf[6];
  int ptyno;
  struct stat st;

#ifdef TIOCGPTN
  static int tiocgptn_works = 1;
  if (tiocgptn_works)
    {
      if (ioctl (fd, TIOCGPTN, &ptyno) == 0)
	goto gotit;
      else
	{
	  if(errno != EINVAL)
	    return 0;
	  else
	    tiocgptn_works = 0;
	}
    }
#endif
  /* /dev/ptmx will make it into the kernel before 32 bit dev_t, so
     this should be safe.  */
  if (__fxstat (_STAT_VER, fd, &st))
    return 0;

  ptyno = minor (st.st_rdev);
  if (major (st.st_rdev) == 4)
    ptyno -= 128;

#ifdef TIOCGPTN
gotit:
#endif
  /* Two different possible naming schemes for pty slaves:
     the SVr4 way.  */

  idbuf[5] = '\0';
  __stpcpy (__stpcpy (nbuf, "/dev/pts/"),
	    _itoa_word (ptyno, &idbuf[4], 10, 0));
  if (!__xstat (_STAT_VER, nbuf, &st))
    return strncpy (buf, nbuf, len);
  else
    if (errno != ENOENT)
      return NULL;

  /* ...and the BSD way.  */
  nbuf[5]  = 't';
  nbuf[7]  = 'y';
  nbuf[8]  = __ptyname1[ptyno / 16];
  nbuf[9]  = __ptyname2[ptyno % 16];
  nbuf[10] = '\0';

  if (__xstat (_STAT_VER, nbuf, &st))
    return NULL;

  return strncpy (buf, nbuf, len);
}
weak_alias (__ptsname_r, ptsname_r)
