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
#include <stdlib.h>
#include <utmp.h>

/* Return the result of ttyname in the buffer pointed to by TTY, which should
   be of length BUF_LEN.  If it is too long to fit in this buffer, a
   sufficiently long buffer is allocated using malloc, and returned in TTY.
   0 is returned upon success, -1 otherwise.  */
static int
tty_name (int fd, char **tty, size_t buf_len)
{
  int rv;			/* Return value.  0 = success.  */
  char *buf = *tty;		/* Buffer for ttyname, initially the user's. */

  for (;;)
    {
      char *new_buf;

      if (buf_len)
	{
	  rv = ttyname_r (fd, buf, buf_len);

	  if (rv < 0 || memchr (buf, '\0', buf_len))
	    /* We either got an error, or we succeeded and the returned name fit
	       in the buffer.  */
	    break;

	  /* Try again with a longer buffer.  */
	  buf_len += buf_len;	/* Double it */
	}
      else
	/* No initial buffer; start out by mallocing one.  */
	buf_len = 128;		/* First time guess.  */

      if (buf != *tty)
	/* We've already malloced another buffer at least once.  */
	new_buf = realloc (buf, buf_len);
      else
	new_buf = malloc (buf_len);
      if (! new_buf)
	{
	  rv = -1;
	  errno = ENOMEM;
	  break;
	}
    }

  if (rv == 0)
    *tty = buf;			/* Return buffer to the user.  */
  else if (buf != *tty)
    free (buf);			/* Free what we malloced when returning an error.  */

  return rv;
}

void
login (const struct utmp *ut)
{
#ifdef PATH_MAX
  char _tty[PATH_MAX + UT_LINESIZE];
#else
  char _tty[512 + UT_LINESIZE];
#endif
  char *tty = _tty;
  int found_tty;
  const char *ttyp;
  struct utmp_data data = { -1 };
  struct utmp copy = *ut;

  /* Fill in those fields we supply.  */
#if _HAVE_UT_TYPE - 0
  copy.ut_type = USER_PROCESS;
#endif
#if _HAVE_UT_PID - 0
  copy.ut_pid = getpid ();
#endif

  /* Seek tty.  */
  found_tty = tty_name (STDIN_FILENO, &tty, sizeof (_tty));
  if (found_tty < 0)
    found_tty = tty_name (STDOUT_FILENO, &tty, sizeof (_tty));
  if (found_tty < 0)
    found_tty = tty_name (STDERR_FILENO, &tty, sizeof (_tty));

  if (found_tty >= 0)
    {
      /* We only want to insert the name of the tty without path.  */
      ttyp = basename (tty);

      /* Position to record for this tty.  */
      strncpy (copy.ut_line, ttyp, UT_LINESIZE);

      /* Tell that we want to use the UTMP file.  */
      if (utmpname (_PATH_UTMP) != 0)
	{
	  struct utmp *old;

	  /* Open UTMP file.  */
	  setutent_r (&data);

	  /* Read the record.  */
	  if (getutline_r (&copy, &old, &data) >= 0)
	    {
#if _HAVE_UT_TYPE - 0
	      /* We have to fake the old entry because this `login'
		 function does not fit well into the UTMP file
		 handling scheme.  */
	      old->ut_type = copy.ut_type;
#endif
	      pututline_r (&copy, &data);
	    }
	  else if (errno == ESRCH)
	    /* We didn't find anything.  pututline_r will add UT at the end
	       of the file in this case.  */
	    pututline_r (&copy, &data);

	  /* Close UTMP file.  */
	  endutent_r (&data);
	}

      if (tty != _tty)
	free (tty);		/* Free buffer malloced by tty_name.  */
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
	  data.ubuf.ut_type = copy.ut_type;
#endif
	  pututline_r (&copy, &data);
	}

      /* Close WTMP file.  */
      endutent_r (&data);
    }
}
