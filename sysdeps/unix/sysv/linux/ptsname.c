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

#include <errno.h>
#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <termios.h>
#include <unistd.h>

#include <stdio-common/_itoa.h>

/* Directory where we can find the slave pty nodes.  */
#define _PATH_DEVPTS "/dev/pts/"

/* The are declared in getpt.c.  */
extern const char __libc_ptyname1[];
extern const char __libc_ptyname2[];

/* Static buffer for `ptsname'.  */
static char buffer[sizeof (_PATH_DEVPTS) + 20];


/* Return the pathname of the pseudo terminal slave assoicated with
   the master FD is open on, or NULL on errors.
   The returned storage is good until the next call to this function.  */
char *
ptsname (int fd)
{
  return __ptsname_r (fd, buffer, sizeof (buffer)) != 0 ? NULL : buffer;
}


/* Store at most BUFLEN characters of the pathname of the slave pseudo
   terminal associated with the master FD is open on in BUF.
   Return 0 on success, otherwise an error number.  */
int
__ptsname_r (int fd, char *buf, size_t buflen)
{
  int save_errno = errno;
  struct stat st;
  int ptyno;

  if (buf == NULL)
    {
      __set_errno (EINVAL);
      return EINVAL;
    }

  if (!__isatty (fd))
    {
      __set_errno (ENOTTY);
      return ENOTTY;
    }

#ifdef TIOCGPTN
  if (__ioctl (fd, TIOCGPTN, &ptyno) == 0)
    {
      /* Buffer we use to print the number in.  For a maximum size for
         `int' of 8 bytes we never need more than 20 digits.  */
      char numbuf[21];
      const char *devpts = _PATH_DEVPTS;
      const size_t devptslen = strlen (devpts);
      char *p;

      numbuf[20] = '\0';
      p = _itoa_word (ptyno, &numbuf[20], 10, 0);

      if (buflen < devptslen + strlen (p) + 1)
	{
	  __set_errno (ERANGE);
	  return ERANGE;
	}

      __stpcpy (__stpcpy (buf, devpts), p);
    }
  else if (errno == EINVAL)
#endif
    {
      char *p;

      if (buflen < strlen (_PATH_TTY) + 3)
	{
	  __set_errno (ERANGE);
	  return ERANGE;
	}

      if (__fstat (fd, &st) < 0)
	return errno;

      ptyno = minor (st.st_rdev);
      if (major (st.st_rdev) == 4)
	ptyno -= 128;

      if (ptyno / 16 >= strlen (__libc_ptyname1))
	{
	  __set_errno (ENOTTY);
	  return ENOTTY;
	}

      p = __stpcpy (buf, _PATH_TTY);
      p[0] = __libc_ptyname1[ptyno / 16];
      p[1] = __libc_ptyname2[ptyno % 16];
      p[2] = '\0';
    }

  if (__xstat (_STAT_VER, buf, &st) < 0)
    return errno;

  __set_errno (save_errno);
  return 0;
}
weak_alias (__ptsname_r, ptsname_r)
