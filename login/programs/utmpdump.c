/* utmpdump - dump utmp-like files.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Mark Kettenis <kettenis@phys.uva.nl>, 1997.

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
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>

static void
print_entry (struct utmp *up)
{
#if _HAVE_UT_TV - 0
  printf ("[%d] [%05d] [%-4.4s] [%-8.8s] [%-12.12s] [%-15.15s] [%ld]\n",
	  up->ut_type, up->ut_pid, up->ut_id, up->ut_user,
	  up->ut_line, 4 + ctime (&up->ut_tv.tv_sec), up->ut_tv.tv_usec);
#else
  printf ("[%d] [%05d] [%-4.4s] [%-8.8s] [%-12.12s] [%-15.15s]\n",
	  up->ut_type, up->ut_pid, up->ut_id, up->ut_user,
	  up->ut_line, 4 + ctime (&up->ut_time));
#endif
}

int
main (int argc, char *argv[])
{
  struct utmp *up;

  if (argc > 1)
    utmpname (argv[1]);

  setutent ();

  while ((up = getutent ()))
    print_entry (up);

  endutent ();

  return EXIT_SUCCESS;
}
