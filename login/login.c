/* Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>

/* XXX used for tty name array in login.  */
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

void
login (const struct utmp *ut)
{
  char tty[PATH_MAX + UT_LINESIZE];
  int found_tty;
  const char *ttyp;
  struct utmp_data data;

  /* Seek tty.  */
  found_tty = ttyname_r (STDIN_FILENO, tty, sizeof tty);
  if (found_tty < 0)
    found_tty = ttyname_r (STDOUT_FILENO, tty, sizeof tty);
  if (found_tty < 0)
    found_tty = ttyname_r (STDERR_FILENO, tty, sizeof tty);

  if (found_tty >= 0)
    {
      /* Tell that we want to use the UTMP file.  */
      if (utmpname (_PATH_UTMP) != 0)
	{
	  struct utmp tmp;
	  struct utmp *old;

	  /* Open UTMP file.  */
	  setutent_r (&data);

	  /* We only want to insert the name of the tty without path.  */
	  ttyp = basename (tty);

	  /* Position to record for this tty.  */
#if _HAVE_UT_TYPE - 0
	  tmp.ut_type = USER_PROCESS;
#endif
	  strncpy (tmp.ut_line, ttyp, UT_LINESIZE);

	  /* Read the record.  */
	  if (getutline_r (&tmp, &old, &data) >= 0 || errno == ESRCH)
	    {
#if _HAVE_UT_TYPE - 0
	      /* We have to fake the old entry because this `login'
		 function does not fit well into the UTMP file
		 handling scheme.  */
	      old->ut_type = ut->ut_type;
#endif
	      pututline_r (ut, &data);
	    }

	  /* Close UTMP file.  */
	  endutent_r (&data);
	}
    }

  /* Update the WTMP file.  Here we have to add a new entry.  */
  if (utmpname (_PATH_WTMP) != 0)
    {
      /* Open the WTMP file.  */
      setutent_r (&data);

      /* Position at end of file.  */
      data.loc_utmp = lseek (data.ut_fd, 0, SEEK_END);
      if (data.loc_utmp != -1)
	{
#if _HAVE_UT_TYPE - 0
	  /* We have to fake the old entry because this `login'
	     function does not fit well into the UTMP file handling
	     scheme.  */
	  data.ubuf.ut_type = ut->ut_type;
#endif
	  pututline_r (ut, &data);
	}

      /* Close WTMP file.  */
      endutent_r (&data);
    }
}
