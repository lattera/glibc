/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Mark Kettenis <kettenis@phys.uva.nl>, 1997.

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

#include <unistd.h>
#include <utmp.h>

#include "utmp-private.h"


void
updwtmp (const char *wtmp_file, const struct utmp *utmp)
{
  /* See whether utmpd is running.  */
  if ((*__libc_utmp_daemon_functions.updwtmp) (wtmp_file, utmp) < 0)
    {
      /* Use the normal file, but if the current file is _PATH_UTMP or
         _PATH_WTMP and the corresponding extended file (with an extra
         'x' added to the pathname) exists, we use the extended file,
         because the original file is in a different format.  */
      if (strcmp (wtmp_file, _PATH_UTMP) == 0
	  && __access (_PATH_UTMP "x", F_OK) == 0)
	(*__libc_utmp_file_functions.updwtmp) (_PATH_UTMP "x", utmp);
      else if (strcmp (wtmp_file, _PATH_WTMP) == 0
	       && __access (_PATH_WTMP "x", F_OK) == 0)
	(*__libc_utmp_file_functions.updwtmp) (_PATH_WTMP "x", utmp);
      else
	(*__libc_utmp_file_functions.updwtmp) (wtmp_file, utmp);
    }
}

