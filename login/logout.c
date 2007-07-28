/* Copyright (C) 1996, 1997, 2002, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <string.h>
#include <utmp.h>
#include <sys/time.h>

int
logout (const char *line)
{
  struct utmp tmp, utbuf;
  struct utmp *ut;
  int result = 0;

  /* Tell that we want to use the UTMP file.  */
  if (utmpname (_PATH_UTMP) == -1)
    return 0;

  /* Open UTMP file.  */
  setutent ();

  /* Fill in search information.  */
#if _HAVE_UT_TYPE - 0
  tmp.ut_type = USER_PROCESS;
#endif
  strncpy (tmp.ut_line, line, sizeof tmp.ut_line);

  /* Read the record.  */
  if (getutline_r (&tmp, &utbuf, &ut) >= 0)
    {
      /* Clear information about who & from where.  */
      bzero (ut->ut_name, sizeof ut->ut_name);
#if _HAVE_UT_HOST - 0
      bzero (ut->ut_host, sizeof ut->ut_host);
#endif
#if _HAVE_UT_TV - 0
      struct timeval tv;
      __gettimeofday (&tv, NULL);
      ut->ut_tv.tv_sec = tv.tv_sec;
      ut->ut_tv.tv_usec = tv.tv_usec;
#else
      ut->ut_time = time (NULL);
#endif
#if _HAVE_UT_TYPE - 0
      ut->ut_type = DEAD_PROCESS;
#endif

      if (pututline (ut) != NULL)
	result = 1;
    }

  /* Close UTMP file.  */
  endutent ();

  return result;
}
