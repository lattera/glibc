/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>

#include <utmp.h>

/* Defined in ttyname.c.  */
extern char *__ttyname;

/* Return the login name of the user, or NULL if it can't be determined.
   The returned pointer, if not NULL, is good only until the next call.  */

char *
DEFUN_VOID(getlogin)
{
  char save_tty_pathname[2 + 2 * NAME_MAX];
  char *save_ttyname;
  char *real_tty_path;
  char *result = NULL;
  FILE *f;
  static struct utmp ut;

  if (__ttyname == NULL)
    save_ttyname = NULL;
  else
    save_ttyname = strcpy (save_tty_pathname, __ttyname);

  {
    int err;
    int d = __open ("/dev/tty", 0);
    if (d < 0)
      return NULL;

    real_tty_path = ttyname (d);
    err = errno;
    (void) close (d);

    if (real_tty_path == NULL)
      {
	errno = err;
	return NULL;
      }
  }

  real_tty_path += 5;		/* Remove "/dev/".  */

  f = fopen ("/etc/utmp", "r");
  if (f != NULL)
    {
      while (fread ((PTR) &ut, sizeof(ut), 1, f) == 1)
	if (!strncmp (ut.ut_line, real_tty_path, sizeof (ut.ut_line)))
	  {
	    result = ut.ut_name;
	    /* The name is not null-terminated if
	       it is as long as sizeof (ut.ut_name).  */
	    result[sizeof (ut.ut_name)] = '\0';
	    break;
	  }
      (void) fclose (f);
    }

  if (save_ttyname != NULL)
    strcpy (__ttyname, save_ttyname);
  if (result == NULL)
    errno = ENOENT;
  return result;
}
