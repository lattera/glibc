/* Reentrant function to return the current login name.  Unix version.
Copyright (C) 1991, 1992, 1996 Free Software Foundation, Inc.
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

/* Return at most NAME_LEN characters of the login name of the user in NAME.
   If it cannot be determined or some other error occured, return the error
   code.  Otherwise return 0.  */

int
getlogin_r (name, name_len)
     char *name;
     size_t name_len;
{
  char tty_pathname[2 + 2 * NAME_MAX];
  char *real_tty_path = tty_pathname;
  int result = 0;
  struct utmp_data utmp_data;
  struct utmp *ut;

  {
    int err;
    int d = __open ("/dev/tty", 0);
    if (d < 0)
      return errno;

    result = ttyname_r (d, real_tty_path, sizeof (tty_pathname));
    err = errno;
    (void) close (d);

    if (result < 0)
      {
	errno = err;
	return err;
      }
  }

  real_tty_path += 5;		/* Remove "/dev/".  */

  setutent_r (&utmp_data);
  if (getutline_r (real_tty_path, &ut, &utmp_data) < 0)
    {
      if (errno == ESRCH)
	/* The caller expects ENOENT if nothing is found.  */
	result = ENOENT;
      else
	result = errno;
    }
  else
    {
      strncpy (name, ut->ut_line, name_len);
      result = 0;
    }
  endutent_r (&utmp_data);

  return result;
}
