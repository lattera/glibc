/* Copyright (C) 1996-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>


void
logwtmp (const char *line, const char *name, const char *host)
{
  struct utmp ut;

  /* Set information in new entry.  */
  memset (&ut, 0, sizeof (ut));
#if _HAVE_UT_PID - 0
  ut.ut_pid = getpid ();
#endif
#if _HAVE_UT_TYPE - 0
  ut.ut_type = name[0] ? USER_PROCESS : DEAD_PROCESS;
#endif
  strncpy (ut.ut_line, line, sizeof ut.ut_line);
  strncpy (ut.ut_name, name, sizeof ut.ut_name);
#if _HAVE_UT_HOST - 0
  strncpy (ut.ut_host, host, sizeof ut.ut_host);
#endif

#if _HAVE_UT_TV - 0
  struct timeval tv;
  __gettimeofday (&tv, NULL);
  ut.ut_tv.tv_sec = tv.tv_sec;
  ut.ut_tv.tv_usec = tv.tv_usec;
#else
  ut.ut_time = time (NULL);
#endif

  updwtmp (_PATH_WTMP, &ut);
}
