/* Copyright (C) 1991, 1992, 1996 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>

#include <utmp.h>

/* Return the login name of the user, or NULL if it can't be determined.
   The returned pointer, if not NULL, is good only until the next call.  */

char *
getlogin (void)
{
  char tty_pathname[2 + 2 * NAME_MAX];
  char *real_tty_path = tty_pathname;
  char *result = NULL;
  static struct utmp_data utmp_data = { ut_fd: -1 };
  struct utmp *ut, line;

  {
    int err = 0;
    int d = __open ("/dev/tty", 0);
    if (d < 0)
      return NULL;

    if (__ttyname_r (d, real_tty_path, sizeof (tty_pathname)) < 0)
      err = errno;
    (void) close (d);

    if (err != 0)
      {
	errno = err;
	return NULL;
      }
  }

  real_tty_path += 5;		/* Remove "/dev/".  */

  __setutent_r (&utmp_data);
  strncpy (line.ut_line, real_tty_path, sizeof line.ut_line);
  if (__getutline_r (&line, &ut, &utmp_data) < 0)
    {
      if (errno == ESRCH)
	/* The caller expects ENOENT if nothing is found.  */
	errno = ENOENT;
      result = NULL;
    }
  else
    result = ut->ut_line;

  __endutent_r (&utmp_data);

  return result;
}
